#pragma once

#include "lvgl.h"

namespace Compose::Test
{
  struct LVGLFixture
  {
    LVGLFixture()
    {
      if(!lv_is_initialized())
        lv_init();

      m_display = lv_display_create(800, 480);
      m_root = lv_obj_create(nullptr);
    }

    ~LVGLFixture()
    {
      lv_obj_delete(m_root);
      lv_display_delete(m_display);
    }

    LVGLFixture(const LVGLFixture &) = delete;
    LVGLFixture &operator=(const LVGLFixture &) = delete;

    lv_display_t *m_display = nullptr;
    lv_obj_t *m_root = nullptr;
  };
}
