#include "DrmScreen.h"

#include <drm_fourcc.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/mman.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include <array>
#include <bit>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <memory>
#include <ranges>
#include <span>
#include <stdexcept>
#include <vector>

namespace Compose
{
  namespace
  {
    static_assert(LV_COLOR_DEPTH == 32 || LV_COLOR_DEPTH == 16);

    constexpr uint32_t c_bitsPerPixel = LV_COLOR_DEPTH;
    constexpr uint32_t c_bytesPerPixel = LV_COLOR_DEPTH / 8;
    constexpr uint32_t c_format = LV_COLOR_DEPTH == 32 ? DRM_FORMAT_XRGB8888 : DRM_FORMAT_RGB565;

    template <typename T> using DrmPtr = std::unique_ptr<T, void (*)(T *)>;

    DrmPtr<drmModeRes> resources(const int fd)
    {
      return { drmModeGetResources(fd), drmModeFreeResources };
    }

    DrmPtr<drmModeConnector> connector(const int fd, const uint32_t id)
    {
      return { drmModeGetConnector(fd, id), drmModeFreeConnector };
    }

    DrmPtr<drmModeEncoder> encoder(const int fd, const uint32_t id)
    {
      return { drmModeGetEncoder(fd, id), drmModeFreeEncoder };
    }

    bool isConnected(const drmModeConnector &candidate)
    {
      return candidate.connection == DRM_MODE_CONNECTED && candidate.count_modes > 0;
    }

    DrmPtr<drmModeConnector> firstConnected(const int fd, const drmModeRes &res)
    {
      DrmPtr<drmModeConnector> result { nullptr, drmModeFreeConnector };

      for(auto id : std::span(res.connectors, res.count_connectors))
        if(!result)
          if(auto candidate = connector(fd, id); candidate && isConnected(*candidate))
            result = std::move(candidate);

      return result;
    }

    void check(const int result, const std::string &what)
    {
      if(result != 0)
        throw std::runtime_error(what + ": " + std::strerror(errno));
    }

    const drmModeModeInfo &modeOf(const drmModeConnector &screen)
    {
      return screen.modes[0];
    }

    uint32_t encoderIdOf(const drmModeConnector &screen)
    {
      if(screen.encoder_id == 0 && screen.count_encoders == 0)
        throw std::runtime_error("no encoder for the connected screen");

      return screen.encoder_id ? screen.encoder_id : screen.encoders[0];
    }

    uint32_t firstPossibleCrtc(const drmModeEncoder &chosen, const drmModeRes &res)
    {
      const auto index = std::countr_zero(chosen.possible_crtcs);
      if(index >= res.count_crtcs)
        throw std::runtime_error("no CRTC for the connected screen");

      return res.crtcs[index];
    }

    uint32_t crtcOf(const int fd, const drmModeConnector &screen, const drmModeRes &res)
    {
      const auto chosen = encoder(fd, encoderIdOf(screen));
      if(!chosen)
        throw std::runtime_error("no encoder for the connected screen");

      return chosen->crtc_id ? chosen->crtc_id : firstPossibleCrtc(*chosen, res);
    }

    struct Output
    {
      uint32_t connector;
      uint32_t crtc;
      drmModeModeInfo mode;
      uint32_t widthMm;
    };

    Output findOutput(const int fd)
    {
      const auto res = resources(fd);
      if(!res)
        throw std::runtime_error(std::string("no DRM resources: ") + std::strerror(errno));

      const auto screen = firstConnected(fd, *res);
      if(!screen)
        throw std::runtime_error("no connected screen");

      return { .connector = screen->connector_id,
               .crtc = crtcOf(fd, *screen, *res),
               .mode = modeOf(*screen),
               .widthMm = screen->mmWidth };
    }

    class Card
    {
     public:
      explicit Card(const std::string &path)
          : m_fd(open(path.c_str(), O_RDWR | O_CLOEXEC))
      {
        if(m_fd < 0)
          throw std::runtime_error("cannot open " + path + ": " + std::strerror(errno));
      }

      ~Card()
      {
        close(m_fd);
      }

      Card(const Card &) = delete;
      Card &operator=(const Card &) = delete;

      int fd() const
      {
        return m_fd;
      }

     private:
      int m_fd;
    };

    class Scanout
    {
     public:
      Scanout(const int fd, const drmModeModeInfo &mode)
          : m_fd(fd)
          , m_dumb(createDumb(fd, mode))
          , m_framebuffer(addFramebuffer(fd, mode, m_dumb))
          , m_pixels(mapDumb(fd, m_dumb))
      {
      }

      ~Scanout()
      {
        munmap(m_pixels, m_dumb.size);
        drmModeRmFB(m_fd, m_framebuffer);
        drm_mode_destroy_dumb destroy { .handle = m_dumb.handle };
        drmIoctl(m_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
      }

      Scanout(const Scanout &) = delete;
      Scanout &operator=(const Scanout &) = delete;

      uint32_t framebuffer() const
      {
        return m_framebuffer;
      }

      void fill(const std::vector<uint8_t> &frame, const size_t rowBytes) const
      {
        for(auto row : std::views::iota(size_t { 0 }, frame.size() / rowBytes))
          std::memcpy(m_pixels + row * m_dumb.pitch, frame.data() + row * rowBytes, rowBytes);
      }

     private:
      static drm_mode_create_dumb createDumb(const int fd, const drmModeModeInfo &mode)
      {
        drm_mode_create_dumb dumb { .height = mode.vdisplay,
                                    .width = mode.hdisplay,
                                    .bpp = c_bitsPerPixel,
                                    .flags = 0,
                                    .handle = 0,
                                    .pitch = 0,
                                    .size = 0 };
        check(drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &dumb), "create dumb buffer");
        return dumb;
      }

      static uint32_t addFramebuffer(const int fd, const drmModeModeInfo &mode, const drm_mode_create_dumb &dumb)
      {
        const std::array<uint32_t, 4> handles { dumb.handle, 0, 0, 0 };
        const std::array<uint32_t, 4> pitches { dumb.pitch, 0, 0, 0 };
        const std::array<uint32_t, 4> offsets { };
        uint32_t id = 0;
        check(drmModeAddFB2(fd, mode.hdisplay, mode.vdisplay, c_format, handles.data(), pitches.data(), offsets.data(),
                            &id, 0),
              "add framebuffer");
        return id;
      }

      static uint8_t *mapDumb(const int fd, const drm_mode_create_dumb &dumb)
      {
        drm_mode_map_dumb map { .handle = dumb.handle, .pad = 0, .offset = 0 };
        check(drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map), "map dumb buffer");

        auto *pixels = mmap(nullptr, dumb.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, static_cast<off_t>(map.offset));
        if(pixels == MAP_FAILED)
          throw std::runtime_error(std::string("mmap dumb buffer: ") + std::strerror(errno));

        return static_cast<uint8_t *>(pixels);
      }

      int m_fd;
      drm_mode_create_dumb m_dumb;
      uint32_t m_framebuffer;
      uint8_t *m_pixels;
    };

    class DrmScreen
    {
     public:
      explicit DrmScreen(const std::string &path)
          : m_card(path)
          , m_output(findOutput(m_card.fd()))
          , m_scanouts { Scanout { m_card.fd(), m_output.mode }, Scanout { m_card.fd(), m_output.mode } }
          , m_frame(static_cast<size_t>(rowBytes()) * height())
      {
        if(drmModeSetCrtc(m_card.fd(), m_output.crtc, m_scanouts[0].framebuffer(), 0, 0, &m_output.connector, 1,
                          &m_output.mode)
           != 0)
          LV_LOG_ERROR("drmModeSetCrtc failed: %s", std::strerror(errno));
      }

      ~DrmScreen()
      {
        waitForFlip();
      }

      DrmScreen(const DrmScreen &) = delete;
      DrmScreen &operator=(const DrmScreen &) = delete;

      uint16_t width() const
      {
        return m_output.mode.hdisplay;
      }

      uint16_t height() const
      {
        return m_output.mode.vdisplay;
      }

      uint32_t widthMm() const
      {
        return m_output.widthMm;
      }

      uint32_t rowBytes() const
      {
        return width() * c_bytesPerPixel;
      }

      std::vector<uint8_t> &frame()
      {
        return m_frame;
      }

      void show()
      {
        waitForFlip();
        m_scanouts[m_back].fill(m_frame, rowBytes());
        flipTo(m_scanouts[m_back]);
        m_back = 1 - m_back;
      }

     private:
      void flipTo(const Scanout &scanout)
      {
        if(drmModePageFlip(m_card.fd(), m_output.crtc, scanout.framebuffer(), DRM_MODE_PAGE_FLIP_EVENT, this) == 0)
          m_flipPending = true;
        else
          LV_LOG_ERROR("drmModePageFlip failed: %s", std::strerror(errno));
      }

      void waitForFlip()
      {
        while(m_flipPending)
          handleNextEvent();
      }

      void handleNextEvent()
      {
        pollfd request { .fd = m_card.fd(), .events = POLLIN, .revents = 0 };
        const auto ready = poll(&request, 1, -1);
        assert(ready > 0 || errno == EINTR);

        if(ready > 0)
          drmHandleEvent(m_card.fd(), &m_events);
      }

      static void onFlipped(int, unsigned int, unsigned int, unsigned int, void *const screen)
      {
        static_cast<DrmScreen *>(screen)->m_flipPending = false;
      }

      Card m_card;
      Output m_output;
      std::array<Scanout, 2> m_scanouts;
      std::vector<uint8_t> m_frame;
      size_t m_back = 1;
      bool m_flipPending = false;
      drmEventContext m_events { .version = 2,
                                 .vblank_handler = nullptr,
                                 .page_flip_handler = onFlipped,
                                 .page_flip_handler2 = nullptr,
                                 .sequence_handler = nullptr };
    };

    DrmScreen &screenOf(lv_display_t *const display)
    {
      return *static_cast<DrmScreen *>(lv_display_get_driver_data(display));
    }

    void flush(lv_display_t *const display, const lv_area_t *, uint8_t *)
    {
      if(lv_display_flush_is_last(display))
        screenOf(display).show();

      lv_display_flush_ready(display);
    }

    void deleteScreen(lv_event_t *const event)
    {
      delete &screenOf(static_cast<lv_display_t *>(lv_event_get_current_target(event)));
    }

    int32_t dotsPerInch(const DrmScreen &screen)
    {
      return static_cast<int32_t>((screen.width() * 25400 + screen.widthMm() * 1000 - 1) / (screen.widthMm() * 1000));
    }
  }

  lv_display_t *createDrmDisplay(const std::string &card)
  {
    auto screen = std::make_unique<DrmScreen>(card);
    auto *display = lv_display_create(screen->width(), screen->height());

    lv_display_set_buffers_with_stride(display, screen->frame().data(), nullptr, screen->frame().size(),
                                       screen->rowBytes(), LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_flush_cb(display, flush);

    if(screen->widthMm())
      lv_display_set_dpi(display, dotsPerInch(*screen));

    lv_display_set_driver_data(display, screen.release());
    lv_display_add_event_cb(display, deleteScreen, LV_EVENT_DELETE, nullptr);
    return display;
  }

  std::optional<Size> drmScreenSize(const std::string &card)
  {
    std::optional<Size> result;

    if(const int fd = open(card.c_str(), O_RDWR | O_CLOEXEC); fd >= 0)
    {
      if(const auto res = resources(fd))
        if(const auto screen = firstConnected(fd, *res))
          result = Size { .w = static_cast<int>(modeOf(*screen).hdisplay),
                          .h = static_cast<int>(modeOf(*screen).vdisplay) };

      close(fd);
    }

    return result;
  }
}
