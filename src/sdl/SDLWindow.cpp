#include <compose/widgets/Window.h>
#include <SDL.h>
#include "src/drivers/sdl/lv_sdl_keyboard.h"
#include "src/drivers/sdl/lv_sdl_mouse.h"
#include "src/drivers/sdl/lv_sdl_mousewheel.h"
#include "src/drivers/sdl/lv_sdl_window.h"
#include "src/display/lv_display.h"
#include "src/indev/lv_indev.h"
#include "tools/Log.h"

#include <algorithm>
#include <vector>

namespace Compose
{
  namespace
  {
    constexpr int c_maxTouchPoints = 5;

    struct TouchPoint
    {
      SDL_FingerID fingerId;
      lv_point_t point;
    };

    struct Snapshot
    {
      uint32_t tick = 0;
      std::vector<TouchPoint> points;
    };

    struct TouchSlot
    {
      Snapshot *snapshot;
      lv_display_t *display;
      int slot;
      lv_point_t lastPoint = { 0, 0 };
      lv_indev_state_t lastState = LV_INDEV_STATE_RELEASED;
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
            snapshot.points.push_back(TouchPoint { finger->id, lv_point_t { x, y } });
          }
        }
      }

      std::ranges::sort(snapshot.points, {}, &TouchPoint::fingerId);
    }

    void readTouchSlot(lv_indev_t *indev, lv_indev_data_t *data)
    {
      auto *slot = static_cast<TouchSlot *>(lv_indev_get_driver_data(indev));
      if(!slot || !slot->snapshot || !slot->display)
      {
        data->state = LV_INDEV_STATE_RELEASED;
        data->point = lv_point_t { 0, 0 };
        return;
      }

      refreshSnapshot(*slot->snapshot, slot->display);

      if(slot->slot < static_cast<int>(slot->snapshot->points.size()))
      {
        const auto &point = slot->snapshot->points[slot->slot].point;
        data->state = LV_INDEV_STATE_PRESSED;
        data->point = point;
        slot->lastPoint = point;
        slot->lastState = data->state;
      }
      else
      {
        data->state = LV_INDEV_STATE_RELEASED;
        data->point = slot->lastPoint;
        slot->lastState = data->state;
      }
    }

    void releaseTouchSlot(lv_event_t *e)
    {
      auto *indev = static_cast<lv_indev_t *>(lv_event_get_user_data(e));
      auto *slot = static_cast<TouchSlot *>(lv_indev_get_driver_data(indev));
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
      lv_indev_set_driver_data(indev, slot);
      lv_indev_set_display(indev, display);
      lv_indev_add_event_cb(indev, releaseTouchSlot, LV_EVENT_DELETE, indev);
      return indev;
    }
  }

  Window::Window(Rect position, Rotation rotation)
  {
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
  }
}
