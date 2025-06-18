#include "compose/widgets/Application.h"
#include "compose/widgets/Window.h"
#include "src/misc/lv_timer.h"
#include "src/tick/lv_tick.h"
#include <lvgl.h>

#include <chrono>
#include <thread>
#include <reactive/Computations.h>

namespace Compose
{
  Application::Application(Window::Backend backend)
      : m_backend(backend)
  {
    lv_init();
  }

  [[noreturn]] void Application::runBlocking(const tCallback& callback) const
  {
    Window window { m_backend };
    const Reactive::Computations c;
    c.add([&] { callback(window); });
    auto lastTick = std::chrono::high_resolution_clock::now();

    while(true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      const auto current = std::chrono::high_resolution_clock::now();
      auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(current - lastTick);
      lv_tick_inc(delta.count());
      lastTick = current;
      lv_timer_handler();
    }
  }
}