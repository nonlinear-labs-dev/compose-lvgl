#include "src/core/lv_obj.h"

#include <compose/widgets/Window.h>
#include "src/drivers/sdl/lv_sdl_keyboard.h"
#include "src/drivers/sdl/lv_sdl_mouse.h"
#include "src/drivers/sdl/lv_sdl_mousewheel.h"
#include "src/drivers/sdl/lv_sdl_window.h"
#include "src/misc/lv_types.h"

#include <SDL.h>
#include <stdexcept>

namespace Compose
{
  Window::Window(Backend endback, Size size)
  {
    switch(endback)
    {
      case Backend::SDL:
        SDL_Init(SDL_INIT_EVERYTHING);
        m_display = lv_sdl_window_create(size.w, size.h);
        lv_sdl_mouse_create();
        lv_sdl_mousewheel_create();
        lv_sdl_keyboard_create();
        break;
      case Backend::Framebuffer:
        throw std::runtime_error("Not implemented");
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