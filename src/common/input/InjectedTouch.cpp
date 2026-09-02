#include "compose/input/InjectedTouch.h"

#include "src/display/lv_display.h"
#include "src/indev/lv_indev.h"

namespace Compose
{
  InjectedTouch::InjectedTouch()
  {
    m_indevData.activeTouchCount = &m_activeTouchCount;
    m_indevData.context = this;

    m_indev = lv_indev_create();
    lv_indev_set_type(m_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(m_indev, read);
    lv_indev_set_driver_data(m_indev, &m_indevData);
    lv_indev_set_display(m_indev, lv_display_get_default());
  }

  InjectedTouch::~InjectedTouch()
  {
    lv_indev_delete(m_indev);
  }

  void InjectedTouch::enqueue(Step step)
  {
    m_steps.push_back(step);
  }

  bool InjectedTouch::isIdle() const
  {
    return m_steps.empty();
  }

  void InjectedTouch::read(lv_indev_t *indev, lv_indev_data_t *data)
  {
    static_cast<InjectedTouch *>(getTouchIndevData(indev)->context)->readNext(data);
  }

  void InjectedTouch::readNext(lv_indev_data_t *data)
  {
    const bool wasPressed = m_current.pressed;

    if(!m_steps.empty())
    {
      m_current = m_steps.front();
      m_steps.pop_front();
    }

    if(m_current.pressed && !wasPressed)
      m_indevData.pointerId = ++m_lastPointerId;

    m_activeTouchCount = m_current.pressed ? 1 : 0;
    data->state = m_current.pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    data->point = lv_point_t { m_current.x, m_current.y };
  }
}
