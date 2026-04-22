#include "TouchGestureFeed.h"

#include <algorithm>

namespace Compose
{
  namespace
  {
    size_t s_activeTouchCount = 0;
  }

  TouchGestureFeed::TouchGestureFeed(size_t maxPoints)
      : m_maxPoints(maxPoints)
  {
  }

  bool TouchGestureFeed::shouldRefresh(uint32_t lvTick) const
  {
    return !m_hasLastReadTick || m_lastReadLvTick != lvTick;
  }

  void TouchGestureFeed::markRefreshed(uint32_t lvTick)
  {
    m_lastReadLvTick = lvTick;
    m_hasLastReadTick = true;
  }

  void TouchGestureFeed::update(const std::vector<TouchSample> &currentTouches, uint32_t timestamp)
  {
    const auto previousPoints = m_points;
    const auto previousTouchCount = previousPoints.size();
    std::vector<TouchSample> clippedTouches;
    clippedTouches.reserve(std::min(currentTouches.size(), m_maxPoints));

    for(const auto &touch : currentTouches)
    {
      if(clippedTouches.size() < m_maxPoints)
      {
        clippedTouches.push_back(touch);
      }
    }

    m_points.clear();
    m_points.reserve(clippedTouches.size());

    for(const auto &oldPoint : previousPoints)
    {
      if(auto it = std::ranges::find(clippedTouches, oldPoint.id, &TouchSample::id); it != clippedTouches.end())
      {
        m_points.push_back(*it);
      }
    }

    for(const auto &touch : clippedTouches)
    {
      const auto isNewTouch = std::ranges::none_of(m_points, [&](const auto &knownTouch) { return knownTouch.id == touch.id; });
      if(isNewTouch)
      {
        m_points.push_back(touch);
      }
    }

    s_activeTouchCount = m_points.size();
    const auto touchCountChanged = previousTouchCount != m_points.size();

    m_gestureData.clear();
    m_gestureData.reserve(m_points.size() + previousPoints.size());

    for(const auto &touch : m_points)
    {
      if(auto it = std::ranges::find(previousPoints, touch.id, &TouchSample::id);
         touchCountChanged || it == previousPoints.end() || it->point.x != touch.point.x || it->point.y != touch.point.y)
      {
        m_gestureData.push_back(lv_indev_touch_data_t {
            .point = touch.point,
            .state = LV_INDEV_STATE_PRESSED,
            .id = static_cast<uint8_t>(touch.id & 0xFF),
            .timestamp = timestamp,
        });
      }
    }

    for(const auto &oldTouch : previousPoints)
    {
      const auto stillPresent = std::ranges::any_of(m_points, [&](const auto &touch) { return touch.id == oldTouch.id; });
      if(!stillPresent)
      {
        m_gestureData.push_back(lv_indev_touch_data_t {
            .point = oldTouch.point,
            .state = LV_INDEV_STATE_RELEASED,
            .id = static_cast<uint8_t>(oldTouch.id & 0xFF),
            .timestamp = timestamp,
        });
      }
    }
  }

  const std::vector<TouchSample> &TouchGestureFeed::points() const
  {
    return m_points;
  }

  std::vector<lv_indev_touch_data_t> &TouchGestureFeed::gestureData()
  {
    return m_gestureData;
  }

  size_t activeTouchCount(lv_indev_t *indev)
  {
    if(auto *touch = static_cast<TouchGestureIndevData *>(lv_indev_get_driver_data(indev)))
    {
      if(touch->magic == TouchGestureIndevData::c_magic && touch->feed)
      {
        return touch->feed->points().size();
      }
    }

    return s_activeTouchCount;
  }

  void applyTouchGestureFeed(lv_indev_t *indev, lv_indev_data_t *data, TouchGestureFeed &feed, lv_point_t &lastPoint)
  {
    if(!feed.points().empty())
    {
      data->state = LV_INDEV_STATE_PRESSED;
      data->point = feed.points().front().point;
      lastPoint = data->point;
    }
    else
    {
      data->state = LV_INDEV_STATE_RELEASED;
      data->point = lastPoint;
    }

    static lv_indev_touch_data_t emptyTouchData {};
    auto &gestureData = feed.gestureData();
    auto *gestureDataPtr = gestureData.empty() ? &emptyTouchData : gestureData.data();
    auto gestureDataSize = static_cast<uint16_t>(gestureData.size());
    lv_indev_gesture_recognizers_update(indev, gestureDataPtr, gestureDataSize);
    gestureData.clear();
    lv_indev_gesture_recognizers_set_data(indev, data);
  }
}
