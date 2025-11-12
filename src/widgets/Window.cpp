#include "src/core/lv_obj.h"

#include <compose/widgets/Window.h>
#include "src/misc/lv_types.h"

#ifndef CROSS_BUILD
#include <SDL.h>
#include "src/drivers/sdl/lv_sdl_keyboard.h"
#include "src/drivers/sdl/lv_sdl_mouse.h"
#include "src/drivers/sdl/lv_sdl_mousewheel.h"
#include "src/drivers/sdl/lv_sdl_window.h"
#else

#endif

#include <stdexcept>

namespace Compose
{
  Window::Window(Backend endback, Size size)
  {
    switch(endback)
    {
      case Backend::SDL:
#ifndef CROSS_BUILD
        SDL_Init(SDL_INIT_EVERYTHING);
        m_display = lv_sdl_window_create(size.w, size.h);
        lv_sdl_mouse_create();
        lv_sdl_mousewheel_create();
        lv_sdl_keyboard_create();
#else
        throw std::runtime_error("Not implemented");
#endif
        break;
      case Backend::Framebuffer:
#ifdef CROSS_BUILD
        lv_display_t *disp = lv_display_create(size.w, size.h);
        lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);

        uint32_t buf_size = size.w * size.h * lv_color_format_get_size(lv_display_get_color_format(disp));
        lv_color_t *draw_buf = (lv_color_t *) malloc(buf_size);
        lv_display_set_buffers(disp, draw_buf, NULL, buf_size, LV_DISPLAY_RENDER_MODE_FULL);
        lv_display_set_flush_cb(disp, lv_display_flush_ready);
        lv_display_set_default(disp);
#else
        throw std::runtime_error("Not implemented");
#endif
    }
  }

  Window::~Window() = default;

  void Window::setFullScreen(const bool f) const
  {
    [[maybe_unused]] auto v = m_display;
    throw std::runtime_error("Not implemented");
  }

  void Window::setSize(int x, int y) const
  {
    lv_display_set_resolution(m_display, x, y);
  }
}