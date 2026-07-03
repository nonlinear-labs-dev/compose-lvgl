#include "compose/widgets/Application.h"
#include <glibmm/main.h>

#include "compose/widgets/Window.h"
#include "src/misc/lv_timer.h"
#include "src/themes/lv_theme_private.h"
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

    void applyScrollbarStyle(lv_theme_t*, lv_obj_t* obj)
    {
      static lv_style_t style = []
      {
        lv_style_t s;
        lv_style_init(&s);
        lv_style_set_bg_color(&s, lv_palette_darken(LV_PALETTE_GREY, 2));
        lv_style_set_bg_opa(&s, LV_OPA_40);
        lv_style_set_radius(&s, LV_RADIUS_CIRCLE);
        lv_style_set_pad_all(&s, 7);
        lv_style_set_width(&s, 5);
        return s;
      }();
      lv_obj_add_style(obj, &style, LV_PART_SCROLLBAR);
    }

    lv_theme_t* scrollbarOnlyTheme()
    {
      static lv_theme_t theme = []
      {
        lv_theme_t t {};
        lv_theme_set_apply_cb(&t, applyScrollbarStyle);
        return t;
      }();
      return &theme;
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

    // The default theme attaches state-selective styles (e.g. scrollbar_scrolled) to every
    // widget, so every SCROLLED/PRESSED state toggle forces a recursive style refresh of the
    // whole subtree. All other styling is done explicitly here anyway, so replace it with a
    // theme that only provides a state-independent scrollbar look.
    lv_display_set_theme(window.getDisplay(), scrollbarOnlyTheme());

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
    [[maybe_unused]] auto leakingDeferrer = new Reactive::Deferrer();
  }
}
