#include <compose/widgets/Window.h>
#include <SDL.h>
#include "src/drivers/sdl/lv_sdl_keyboard.h"
#include "src/drivers/sdl/lv_sdl_mouse.h"
#include "src/drivers/sdl/lv_sdl_mousewheel.h"
#include "src/drivers/sdl/lv_sdl_window.h"
#include "src/display/lv_display.h"
#include "src/indev/lv_indev.h"
#include <compose/input/TouchIndev.h>

#include <algorithm>
#include <unordered_map>
#include <vector>

namespace Compose
{
  namespace
  {
    constexpr int c_maxTouchPoints = 5;

    struct TouchPoint
    {
      SDL_FingerID fingerId;
      uint32_t pointerId = 0;
      lv_point_t point;
    };

    struct Snapshot
    {
      uint32_t tick = 0;
      size_t activeTouchCount = 0;
      uint32_t lastTouchTick = 0;
      uint32_t nextPointerId = 1;
      std::unordered_map<SDL_FingerID, uint32_t> pointerIds;
      std::vector<TouchPoint> points;
    };

    struct TouchSlot
    {
      TouchIndevData common;
      Snapshot *snapshot = nullptr;
      lv_display_t *display = nullptr;
      int slot = 0;
      uint32_t currentPointerId = 0;
      lv_point_t lastPoint = { 0, 0 };
    };

    void refreshSnapshot(Snapshot &snapshot, lv_display_t *display)
    {
      const auto currentTick = SDL_GetTicks();
      if(snapshot.tick == currentTick)
      {
        return;
      }

      snapshot.tick = currentTick;
      snapshot.points.clear();
      SDL_PumpEvents();

      const auto offsetX = lv_display_get_offset_x(display);
      const auto offsetY = lv_display_get_offset_y(display);
      const auto width = lv_display_get_horizontal_resolution(display);
      const auto height = lv_display_get_vertical_resolution(display);

      const auto numTouchDevices = SDL_GetNumTouchDevices();
      for(int deviceIndex = 0; deviceIndex < numTouchDevices; deviceIndex++)
      {
        const auto touchId = SDL_GetTouchDevice(deviceIndex);
        const auto numFingers = SDL_GetNumTouchFingers(touchId);
        for(int fingerIndex = 0; fingerIndex < numFingers; fingerIndex++)
        {
          if(auto finger = SDL_GetTouchFinger(touchId, fingerIndex))
          {
            const auto x = std::clamp<int>(offsetX + static_cast<int>(finger->x * width), offsetX, offsetX + width - 1);
            const auto y = std::clamp<int>(offsetY + static_cast<int>(finger->y * height), offsetY, offsetY + height - 1);
            const auto [it, inserted] = snapshot.pointerIds.try_emplace(finger->id, snapshot.nextPointerId);
            if(inserted)
            {
              snapshot.nextPointerId++;
            }

            snapshot.points.push_back(TouchPoint { .fingerId = finger->id, .pointerId = it->second, .point = { x, y } });
          }
        }
      }

      std::ranges::sort(snapshot.points, {}, &TouchPoint::fingerId);

      snapshot.activeTouchCount = snapshot.points.size();

      if(!snapshot.points.empty())
        snapshot.lastTouchTick = currentTick;
    }

    void readTouchSlot(lv_indev_t *indev, lv_indev_data_t *data)
    {
      auto *common = static_cast<TouchIndevData *>(lv_indev_get_driver_data(indev));
      auto *slot = common ? static_cast<TouchSlot *>(common->context) : nullptr;
      if(!slot || !slot->snapshot || !slot->display)
      {
        data->state = LV_INDEV_STATE_RELEASED;
        data->point = lv_point_t { 0, 0 };
        return;
      }

      refreshSnapshot(*slot->snapshot, slot->display);

      if(slot->slot < static_cast<int>(slot->snapshot->points.size()))
      {
        const auto &touchPoint = slot->snapshot->points[slot->slot];
        slot->currentPointerId = touchPoint.pointerId;
        slot->common.pointerId = slot->currentPointerId;
        data->state = LV_INDEV_STATE_PRESSED;
        data->point = touchPoint.point;
        slot->lastPoint = touchPoint.point;
      }
      else
      {
        slot->common.pointerId = slot->currentPointerId;
        data->state = LV_INDEV_STATE_RELEASED;
        data->point = slot->lastPoint;
      }
    }

    void releaseTouchSlot(lv_event_t *e)
    {
      auto *indev = static_cast<lv_indev_t *>(lv_event_get_user_data(e));
      auto *common = static_cast<TouchIndevData *>(lv_indev_get_driver_data(indev));
      auto *slot = common ? static_cast<TouchSlot *>(common->context) : nullptr;
      if(slot)
      {
        lv_indev_set_driver_data(indev, nullptr);
        lv_indev_set_read_cb(indev, nullptr);
        lv_free(slot);
      }
    }

    lv_indev_t *createTouchIndev(Snapshot *snapshot, lv_display_t *display, int slotIndex)
    {
      auto *slot = static_cast<TouchSlot *>(lv_malloc_zeroed(sizeof(TouchSlot)));
      if(!slot)
      {
        return nullptr;
      }

      slot->common.magic = TouchIndevData::c_magic;
      slot->common.activeTouchCount = &snapshot->activeTouchCount;
      slot->common.context = slot;
      slot->snapshot = snapshot;
      slot->display = display;
      slot->slot = slotIndex;

      auto *indev = lv_indev_create();
      if(!indev)
      {
        lv_free(slot);
        return nullptr;
      }

      lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
      lv_indev_set_read_cb(indev, readTouchSlot);
      lv_indev_set_driver_data(indev, &slot->common);
      lv_indev_set_display(indev, display);
      lv_indev_add_event_cb(indev, releaseTouchSlot, LV_EVENT_DELETE, indev);
      return indev;
    }
  }

  namespace
  {
    // The desktop drives the mouse pointer along with touchscreen input, so a
    // tap would reach lvgl twice: via the touch indevs and again as a mouse
    // click. Mute the mouse while fingers are down (plus a grace period, since
    // the derived mouse events may trail the touch).
    constexpr uint32_t c_mouseSuppressionAfterTouchMs = 150;
    lv_indev_read_cb_t s_mouseRead = nullptr;
    lv_indev_t *s_realMouse = nullptr;
    Snapshot *s_mouseFilterSnapshot = nullptr;
    lv_display_t *s_mouseFilterDisplay = nullptr;

    void readMouseSuppressedDuringTouch(lv_indev_t *, lv_indev_data_t *data)
    {
      s_mouseRead(s_realMouse, data);

      const bool wasPressed = data->state == LV_INDEV_STATE_PRESSED;

      auto &snapshot = *s_mouseFilterSnapshot;
      refreshSnapshot(snapshot, s_mouseFilterDisplay);

      const auto sinceTouch = SDL_GetTicks() - snapshot.lastTouchTick;
      if(snapshot.activeTouchCount > 0 || (snapshot.lastTouchTick != 0 && sinceTouch < c_mouseSuppressionAfterTouchMs))
      {
        data->state = LV_INDEV_STATE_RELEASED;
      }
      else if(wasPressed)
      {
        static uint32_t lastLog = 0;
        if(SDL_GetTicks() - lastLog > 500)
        {
          lastLog = SDL_GetTicks();
        }
      }
    }
  }

  Window::Window(Rect position, Rotation rotation)
  {
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
    SDL_Init(SDL_INIT_EVERYTHING);
    m_display = lv_sdl_window_create(position.size.w, position.size.h);
    m_mouse = lv_sdl_mouse_create();
    m_mouseWheel = lv_sdl_mousewheel_create();
    m_keyboard = lv_sdl_keyboard_create();

    static Snapshot snapshot;
    for(int slotIndex = 0; slotIndex < c_maxTouchPoints; slotIndex++)
    {
      if(auto *touchIndev = createTouchIndev(&snapshot, m_display, slotIndex))
      {
        m_touchIndevs.push_back(touchIndev);
      }
    }

    // lv_sdl_mouse_handler locates its indev by comparing the read cb against
    // its private sdl_mouse_read, so that cb must stay untouched. Disable the
    // SDL mouse indev instead (the handler still maintains its state) and let a
    // separate filter indev poll that state through the original read cb.
    s_mouseFilterSnapshot = &snapshot;
    s_mouseFilterDisplay = m_display;
    s_realMouse = m_mouse;
    s_mouseRead = lv_indev_get_read_cb(m_mouse);
    lv_indev_enable(m_mouse, false);

    m_filteredMouse = lv_indev_create();
    lv_indev_set_type(m_filteredMouse, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(m_filteredMouse, readMouseSuppressedDuringTouch);
    lv_indev_set_display(m_filteredMouse, m_display);
  }
}
