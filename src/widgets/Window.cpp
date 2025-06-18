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
  Window::Window(Backend endback)
  {
    switch(endback)
    {
      case Backend::SDL:
        SDL_Init(SDL_INIT_EVERYTHING);
        m_display = lv_sdl_window_create(100, 100);
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

  Container &&Window::add(Container &&it) const
  {
    auto scr = it.getHandle();
    lv_obj_set_size(scr, 100, 100);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);
    lv_screen_load(scr);
    return std::move(it);
  }
}