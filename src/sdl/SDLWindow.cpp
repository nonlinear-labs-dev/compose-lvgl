#include <compose/widgets/Window.h>
#include <SDL.h>
#include "src/drivers/sdl/lv_sdl_keyboard.h"
#include "src/drivers/sdl/lv_sdl_mouse.h"
#include "src/drivers/sdl/lv_sdl_mousewheel.h"
#include "src/drivers/sdl/lv_sdl_window.h"
#include "src/display/lv_display.h"
#include "src/indev/lv_indev.h"
#include "src/indev/lv_indev_gesture.h"
#include "../common/input/TouchGestureFeed.h"

#include <algorithm>
#include <vector>

namespace Compose
{
  namespace
  {
    constexpr int c_maxTouchPoints = 5;

    struct TouchInput
    {
      TouchGestureIndevData common;
      lv_display_t *display;
    };

    std::vector<TouchSample> collectTouches(lv_display_t *display)
    {
      SDL_PumpEvents();
      std::vector<TouchSample> touches;
      touches.reserve(c_maxTouchPoints);

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
            if(touches.size() >= c_maxTouchPoints)
            {
              break;
            }

            const auto x = std::clamp<int>(offsetX + static_cast<int>(finger->x * width), offsetX, offsetX + width - 1);
            const auto y = std::clamp<int>(offsetY + static_cast<int>(finger->y * height), offsetY, offsetY + height - 1);
            touches.push_back(TouchSample { .id = static_cast<uint64_t>(finger->id), .point = { x, y } });
          }
        }
      }
      return touches;
    }

    void readTouch(lv_indev_t *indev, lv_indev_data_t *data)
    {
      auto *common = static_cast<TouchGestureIndevData *>(lv_indev_get_driver_data(indev));
      auto *touch = common ? static_cast<TouchInput *>(common->context) : nullptr;
      if(!touch || !common->feed || !touch->display)
      {
        data->state = LV_INDEV_STATE_RELEASED;
        data->point = lv_point_t { 0, 0 };
        return;
      }

      const auto lvTick = lv_tick_get();
      if(common->feed->shouldRefresh(lvTick))
      {
        auto touches = collectTouches(touch->display);
        common->feed->update(touches, SDL_GetTicks());
        common->feed->markRefreshed(lvTick);
      }

      applyTouchGestureFeed(indev, data, *common->feed, common->lastPoint);
    }

    void releaseTouch(lv_event_t *e)
    {
      auto *indev = static_cast<lv_indev_t *>(lv_event_get_user_data(e));
      auto *touch = static_cast<TouchGestureIndevData *>(lv_indev_get_driver_data(indev));
      if(touch)
      {
        lv_indev_set_driver_data(indev, nullptr);
        lv_indev_set_read_cb(indev, nullptr);
        lv_free(touch->context);
      }
    }

    lv_indev_t *createTouchIndev(TouchGestureFeed *feed, lv_display_t *display)
    {
      auto *touch = static_cast<TouchInput *>(lv_malloc_zeroed(sizeof(TouchInput)));
      if(!touch)
      {
        return nullptr;
      }

      touch->common.feed = feed;
      touch->common.context = touch;
      touch->display = display;

      auto *indev = lv_indev_create();
      if(!indev)
      {
        lv_free(touch);
        return nullptr;
      }

      lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
      lv_indev_set_read_cb(indev, readTouch);
      lv_indev_set_driver_data(indev, &touch->common);
      lv_indev_set_display(indev, display);
      lv_indev_set_pinch_up_threshold(indev, 1.05f);
      lv_indev_set_pinch_down_threshold(indev, 0.95f);
      lv_indev_add_event_cb(indev, releaseTouch, LV_EVENT_DELETE, indev);
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

    static TouchGestureFeed feed { c_maxTouchPoints };
    if(auto *touchIndev = createTouchIndev(&feed, m_display))
    {
      m_touchIndevs.push_back(touchIndev);
    }

  }
}
