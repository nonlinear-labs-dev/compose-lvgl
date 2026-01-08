#include "compose/widgets/Application.h"
#include <glibmm/main.h>

#include "compose/widgets/Window.h"
#include "src/misc/lv_timer.h"
#include "src/tick/lv_tick.h"
#include <lvgl.h>

#include <chrono>
#include <thread>
#include <reactive/Computations.h>

namespace Compose
{
  Application::Application(Rect position, Rotation rotation)
      : m_position(position)
      , m_rotation(rotation)
  {
    lv_init();
    lv_fs_posix_init();
  }

  void Application::runBlocking(const tCallback& callback) const
  {
    Window window { m_position, m_rotation };

    const Reactive::Computations c;
    c.add([&] { callback(window); });

    auto lastTick = std::chrono::high_resolution_clock::now();

    Glib::signal_timeout().connect(
        [&] {
          const auto current = std::chrono::high_resolution_clock::now();
          const auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(current - lastTick);
          lv_tick_inc(delta.count());
          lastTick = current;
          lv_timer_handler();
          return true;
        },
        5);

    auto loop = Glib::MainLoop::create();
    loop->run();
  }
}
