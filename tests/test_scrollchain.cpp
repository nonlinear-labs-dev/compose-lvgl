#include "compose/widgets/Widget.h"
#include "lvgl.h"
#include "TestFinger.h"

#include <catch2/catch_all.hpp>

namespace
{
  struct LvglRuntime
  {
    LvglRuntime()
    {
      if(!lv_is_initialized())
        lv_init();
    }
  };

  struct ScrollablePageFixture : LvglRuntime
  {
    ScrollablePageFixture()
    {
      lv_screen_load(m_root);
      lv_obj_set_size(m_page, 400, 200);
      lv_obj_set_size(m_content, 300, 1000);
      lv_obj_set_size(m_child, 200, 100);
      lv_obj_update_layout(m_root);
    }

    ~ScrollablePageFixture()
    {
      lv_obj_delete(m_root);
      lv_display_delete(m_display);
    }

    ScrollablePageFixture(const ScrollablePageFixture &) = delete;
    ScrollablePageFixture &operator=(const ScrollablePageFixture &) = delete;

    int32_t pageScrollAfterVerticalSwipeOnChild()
    {
      TestFinger finger;
      finger.pressAt({ .x = 50, .y = 80 });
      for(int32_t y = 75; y >= 0; y -= 5)
        finger.moveTo({ .x = 50, .y = y });
      return lv_obj_get_scroll_y(m_page);
    }

    lv_display_t *m_display = lv_display_create(800, 480);
    lv_obj_t *m_root = lv_obj_create(nullptr);
    lv_obj_t *m_page = lv_obj_create(m_root);
    lv_obj_t *m_content = lv_obj_create(m_page);
    lv_obj_t *m_child = lv_obj_create(m_content);
  };
}

TEST_CASE_METHOD(ScrollablePageFixture, "A swipe on a chained child scrolls its page", "[ScrollChain]")
{
  Compose::Widget(m_child).setModifier(Compose::ScrollChain { Compose::ScrollChain::CHAINED });

  CHECK(pageScrollAfterVerticalSwipeOnChild() > 0);
}

TEST_CASE_METHOD(ScrollablePageFixture, "A swipe on an isolated child leaves its page in place", "[ScrollChain]")
{
  Compose::Widget(m_child).setModifier(Compose::ScrollChain { Compose::ScrollChain::ISOLATED });

  CHECK(pageScrollAfterVerticalSwipeOnChild() == 0);
}
