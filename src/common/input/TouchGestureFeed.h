#pragma once

#include "src/indev/lv_indev.h"
#include "src/indev/lv_indev_gesture.h"

#include <cstdint>
#include <vector>

namespace Compose
{
  class TouchGestureFeed;

  struct TouchGestureIndevData
  {
    static constexpr uint32_t c_magic = 0x54475549;

    uint32_t magic = c_magic;
    TouchGestureFeed *feed = nullptr;
    lv_point_t lastPoint = { 0, 0 };
    void *context = nullptr;
  };

  struct TouchSample
  {
    uint64_t id = 0;
    lv_point_t point = { 0, 0 };
  };

  class TouchGestureFeed
  {
   public:
    explicit TouchGestureFeed(size_t maxPoints);

    bool shouldRefresh(uint32_t lvTick) const;
    void markRefreshed(uint32_t lvTick);
    void update(const std::vector<TouchSample> &currentTouches, uint32_t timestamp);

    const std::vector<TouchSample> &points() const;
    std::vector<lv_indev_touch_data_t> &gestureData();

   private:
    size_t m_maxPoints = 0;
    uint32_t m_lastReadLvTick = 0;
    bool m_hasLastReadTick = false;
    std::vector<TouchSample> m_points;
    std::vector<lv_indev_touch_data_t> m_gestureData;
  };

  void applyTouchGestureFeed(lv_indev_t *indev, lv_indev_data_t *data, TouchGestureFeed &feed, lv_point_t &lastPoint);
  size_t activeTouchCount(lv_indev_t *indev);
}
