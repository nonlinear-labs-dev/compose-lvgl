#include "compose/input/InjectedTouch.h"

#include "lvgl.h"
#include "src/display/lv_display.h"
#include "src/indev/lv_indev.h"

#include <algorithm>

namespace Compose
{
  InjectedTouch::InjectedTouch()
  {
    for(auto &finger : m_fingers)
    {
      finger.owner = this;
      finger.indevData.activeTouchCount = &m_activeTouchCount;
      finger.indevData.context = &finger;

      finger.indev = lv_indev_create();
      lv_indev_set_type(finger.indev, LV_INDEV_TYPE_POINTER);
      lv_indev_set_read_cb(finger.indev, read);
      lv_indev_set_driver_data(finger.indev, &finger.indevData);
      lv_indev_set_display(finger.indev, lv_display_get_default());
    }
  }

  InjectedTouch::~InjectedTouch()
  {
    if(lv_is_initialized())
      for(auto &finger : m_fingers)
        lv_indev_delete(finger.indev);
  }

  void InjectedTouch::enqueue(Step step)
  {
    enqueue(0, step);
  }

  void InjectedTouch::enqueue(size_t finger, Step step)
  {
    if(finger < m_fingers.size())
      m_fingers[finger].steps.push_back(step);
  }

  void InjectedTouch::read(lv_indev_t *indev, lv_indev_data_t *data)
  {
    auto *finger = static_cast<Finger *>(getTouchIndevData(indev)->context);
    finger->owner->readNext(*finger, data);
  }

  void InjectedTouch::readNext(Finger &finger, lv_indev_data_t *data)
  {
    const bool wasPressed = finger.current.pressed;

    if(!finger.steps.empty())
    {
      finger.current = finger.steps.front();
      finger.steps.pop_front();
    }

    if(finger.current.pressed && !wasPressed)
      finger.indevData.pointerId = --m_lastPointerId;

    countActiveTouches();
    data->state = finger.current.pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    data->point = lv_point_t { finger.current.x, finger.current.y };
  }

  void InjectedTouch::countActiveTouches()
  {
    m_activeTouchCount = std::ranges::count_if(m_fingers, [](const Finger &finger) { return finger.current.pressed; });
  }
}
