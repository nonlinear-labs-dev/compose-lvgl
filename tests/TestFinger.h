#pragma once

#include "lvgl.h"

class TestFinger
{
 public:
  TestFinger()
      : m_indev(lv_indev_create())
  {
    lv_indev_set_type(m_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_user_data(m_indev, this);
    lv_indev_set_read_cb(m_indev, read);
  }

  ~TestFinger()
  {
    lv_indev_delete(m_indev);
  }

  TestFinger(const TestFinger &) = delete;
  TestFinger &operator=(const TestFinger &) = delete;

  void pressAt(const lv_point_t point)
  {
    m_state = LV_INDEV_STATE_PRESSED;
    moveTo(point);
  }

  void moveTo(const lv_point_t point)
  {
    m_point = point;
    lv_indev_read(m_indev);
  }

 private:
  static void read(lv_indev_t *indev, lv_indev_data_t *data)
  {
    const auto *self = static_cast<TestFinger *>(lv_indev_get_user_data(indev));
    data->point = self->m_point;
    data->state = self->m_state;
  }

  lv_indev_t *m_indev;
  lv_point_t m_point { .x = 0, .y = 0 };
  lv_indev_state_t m_state = LV_INDEV_STATE_RELEASED;
};
