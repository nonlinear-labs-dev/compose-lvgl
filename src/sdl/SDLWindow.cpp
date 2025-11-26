#include <compose/widgets/Window.h>
#include <SDL.h>
#include "src/drivers/sdl/lv_sdl_keyboard.h"
#include "src/drivers/sdl/lv_sdl_mouse.h"
#include "src/drivers/sdl/lv_sdl_mousewheel.h"
#include "src/drivers/sdl/lv_sdl_window.h"

namespace Compose
{
  Window::Window(Size size)
  {
    SDL_Init(SDL_INIT_EVERYTHING);
    m_display = lv_sdl_window_create(size.w, size.h);
    lv_sdl_mouse_create();
    lv_sdl_mousewheel_create();
    lv_sdl_keyboard_create();
  }
}