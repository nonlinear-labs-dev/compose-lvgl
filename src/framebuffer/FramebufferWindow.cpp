#include <compose/widgets/Window.h>
#include <lvgl.h>
#include "src/drivers/display/fb/lv_linux_fbdev.h"
#include "src/drivers/evdev/lv_evdev.h"
#include "src/display/lv_display.h"
#include "src/indev/lv_indev.h"

#include <array>
#include <chrono>
#include <cstdio>
#include <fcntl.h>
#include <linux/input.h>
#include <optional>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <cstdio>

namespace Compose
{

  std::optional<std::string> discoverTouchEvdevPath()
  {
    constexpr int c_maxEventNodes = 64;

    constexpr uint64_t kAbsXyMask = (1ULL << ABS_X) | (1ULL << ABS_Y);
    constexpr uint64_t kMtXyMask = (1ULL << ABS_MT_POSITION_X) | (1ULL << ABS_MT_POSITION_Y);

    for(int i = 0; i < c_maxEventNodes; i++)
    {
      char path[48];
      std::snprintf(path, sizeof(path), "/dev/input/event%d", i);
      const int fd = open(path, O_RDONLY | O_NONBLOCK);
      if(fd < 0)
      {
        continue;
      }

      uint64_t absBits = 0;
      if(ioctl(fd, EVIOCGBIT(EV_ABS, static_cast<int>(sizeof(absBits))), &absBits) < 0)
      {
        close(fd);
        continue;
      }
      close(fd);

      const bool hasClassicXy = (absBits & kAbsXyMask) == kAbsXyMask;
      const bool hasMtXy = (absBits & kMtXyMask) == kMtXyMask;
      if(hasClassicXy || hasMtXy)
      {
        return std::string(path);
      }
    }

    return std::nullopt;
  }

  Window::Window(Rect position, Rotation rotation)
  {
    lv_disp_t *disp = lv_linux_fbdev_create();
    lv_linux_fbdev_set_file(disp, "/dev/fb0");

    if(const auto path = discoverTouchEvdevPath())
    {
      lv_indev_t *indev = lv_evdev_create(LV_INDEV_TYPE_POINTER, path.value().c_str());
      lv_indev_set_display(indev, disp);
    }

    lv_display_set_offset(disp, position.pos.x, position.pos.y);
    lv_display_set_resolution(disp, position.size.w, position.size.h);
    lv_display_set_rotation(disp, static_cast<lv_display_rotation_t>(rotation));

    m_display = disp;

    auto *mtState = new MultiTouchState {};
    mtState->display = disp;

    const auto touchPath = discoverTouchEvdevPath();
    const char *touchDevice = touchPath ? touchPath->c_str() : nullptr;

    if(touchDevice)
    {
      mtState->fd = open(touchDevice, O_RDONLY | O_NONBLOCK);
    }

    if(mtState->fd >= 0)
    {
      input_absinfo absX {};
      input_absinfo absY {};
      if(ioctl(mtState->fd, EVIOCGABS(ABS_MT_POSITION_X), &absX) == 0 && ioctl(mtState->fd, EVIOCGABS(ABS_MT_POSITION_Y), &absY) == 0)
      {
        mtState->minX = absX.minimum;
        mtState->maxX = absX.maximum;
        mtState->minY = absY.minimum;
        mtState->maxY = absY.maximum;
        mtState->hasCalibration = true;
      }
      else if(ioctl(mtState->fd, EVIOCGABS(ABS_X), &absX) == 0 && ioctl(mtState->fd, EVIOCGABS(ABS_Y), &absY) == 0)
      {
        mtState->minX = absX.minimum;
        mtState->maxX = absX.maximum;
        mtState->minY = absY.minimum;
        mtState->maxY = absY.maximum;
        mtState->hasCalibration = true;
      }

      for(int slotIndex = 0; slotIndex < c_maxTouchPoints; slotIndex++)
      {
        if(auto *touchIndev = createTouchIndev(mtState, slotIndex))
        {
          m_touchIndevs.push_back(touchIndev);
        }
      }

      if(m_touchIndevs.empty() && touchDevice)
      {
        m_mouse = lv_evdev_create(LV_INDEV_TYPE_POINTER, touchDevice);
        lv_indev_set_display(m_mouse, disp);
      }
    }
    else if(touchDevice)
    {
      m_mouse = lv_evdev_create(LV_INDEV_TYPE_POINTER, touchDevice);
      lv_indev_set_display(m_mouse, disp);
    }

    m_backendCleanup = [mtState]
    {
      if(mtState->fd >= 0)
      {
        close(mtState->fd);
      }
      delete mtState;
    };
  }
}
