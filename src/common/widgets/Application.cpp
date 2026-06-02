#include "compose/widgets/Application.h"
#include <glibmm/main.h>

#include "compose/widgets/Window.h"
#include "src/misc/lv_timer.h"
#include "src/tick/lv_tick.h"
#include <lvgl.h>

#include <chrono>
#include <reactive/Computations.h>
#include <reactive/Deferrer.h>

namespace Compose
{
  namespace
  {
    std::unique_ptr<Reactive::Deferrer> s_timerDeferrer;

    void flushBeforeRefresh(lv_event_t*)
    {
      s_timerDeferrer.reset();
    }
  }

  Application::Application(Rect position, Rotation rotation)
      : m_position(position)
      , m_rotation(rotation)
  {
    lv_init();
    lv_fs_posix_init();
  }

  void Application::runBlocking(const tCallback& callback) const
  {
    constexpr auto c_frameIntervalInMs = 16;

    Window window { m_position, m_rotation };
    lv_display_add_event_cb(window.getDisplay(), flushBeforeRefresh, LV_EVENT_REFR_START, nullptr);

    const Reactive::Computations c;
    c.add([&] { callback(window); });

    auto lastTick = std::chrono::high_resolution_clock::now();
    const auto loop = Glib::MainLoop::create();

    Glib::signal_timeout().connect(
        [&, loop]
        {
          const auto current = std::chrono::high_resolution_clock::now();
          const auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(current - lastTick);
          lv_tick_inc(delta.count());
          lastTick = current;
          s_timerDeferrer = std::make_unique<Reactive::Deferrer>();
          lv_timer_handler();
          s_timerDeferrer.reset();

          auto keepRunning = lv_display_get_next(nullptr) != nullptr;
          if(!keepRunning)
          {
            loop->quit();
          }

          return keepRunning;
        },
        c_frameIntervalInMs);

    loop->run();
  }
}
