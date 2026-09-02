#pragma once

#include "compose/input/TouchIndev.h"
#include "src/misc/lv_types.h"

#include <cstdint>
#include <deque>

namespace Compose
{
  class InjectedTouch
  {
   public:
    struct Step
    {
      bool pressed = false;
      int32_t x = 0;
      int32_t y = 0;
    };

    InjectedTouch();
    ~InjectedTouch();

    InjectedTouch(const InjectedTouch &) = delete;
    InjectedTouch &operator=(const InjectedTouch &) = delete;

    void enqueue(Step step);
    [[nodiscard]] bool isIdle() const;

   private:
    static void read(lv_indev_t *indev, lv_indev_data_t *data);
    void readNext(lv_indev_data_t *data);

    TouchIndevData m_indevData;
    std::deque<Step> m_steps;
    size_t m_activeTouchCount = 0;
    Step m_current;
    uint32_t m_lastPointerId = 0;
    lv_indev_t *m_indev = nullptr;
  };
}
