#pragma once

#include "compose/input/TouchIndev.h"
#include "src/misc/lv_types.h"

#include <array>
#include <cstdint>
#include <deque>
#include <limits>

namespace Compose
{
  class InjectedTouch
  {
   public:
    static constexpr size_t c_maxFingers = 3;

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
    void enqueue(size_t finger, Step step);

   private:
    struct Finger
    {
      TouchIndevData indevData;
      InjectedTouch *owner = nullptr;
      std::deque<Step> steps;
      Step current;
      lv_indev_t *indev = nullptr;
    };

    static void read(lv_indev_t *indev, lv_indev_data_t *data);
    void readNext(Finger &finger, lv_indev_data_t *data);
    void countActiveTouches();

    std::array<Finger, c_maxFingers> m_fingers;
    size_t m_activeTouchCount = 0;
    uint32_t m_lastPointerId = std::numeric_limits<uint32_t>::max() - 1;
  };
}
