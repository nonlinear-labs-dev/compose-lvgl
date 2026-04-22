#include <compose/widgets/Window.h>
#include <lvgl.h>
#include "src/drivers/display/fb/lv_linux_fbdev.h"
#include "src/drivers/evdev/lv_evdev.h"
#include "src/display/lv_display.h"
#include "src/indev/lv_indev.h"
#include "src/indev/lv_indev_gesture.h"
#include "../common/input/TouchGestureFeed.h"

#include <array>
#include <chrono>
#include <cstdio>
#include <fcntl.h>
#include <linux/input.h>
#include <optional>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>

namespace Compose
{
  namespace
  {
    constexpr int c_maxTouchPoints = 5;
    constexpr int c_maxEventNodes = 64;

    std::optional<std::string> discoverTouchEvdevPath()
    {
      constexpr uint64_t kAbsXyMask = (1ULL << ABS_X) | (1ULL << ABS_Y);
      constexpr uint64_t kMtXyMask = (1ULL << ABS_MT_POSITION_X) | (1ULL << ABS_MT_POSITION_Y);

      for(int i = 0; i < c_maxEventNodes; i++)
      {
        char path[48];
        std::snprintf(path, sizeof(path), "/dev/input/event%d", i);
        const int fd = open(path, O_RDONLY | O_NONBLOCK);
        if(fd < 0)
        {
          continue;
        }

        uint64_t absBits = 0;
        if(ioctl(fd, EVIOCGBIT(EV_ABS, static_cast<int>(sizeof(absBits))), &absBits) < 0)
        {
          close(fd);
          continue;
        }
        close(fd);

        const bool hasClassicXy = (absBits & kAbsXyMask) == kAbsXyMask;
        const bool hasMtXy = (absBits & kMtXyMask) == kMtXyMask;
        if(hasClassicXy || hasMtXy)
        {
          return std::string(path);
        }
      }

      return std::nullopt;
    }

    struct SlotState
    {
      bool active = false;
      int rawX = 0;
      int rawY = 0;
    };

    struct MultiTouchState
    {
      int fd = -1;
      lv_display_t *display = nullptr;
      int currentSlot = 0;
      int minX = 0;
      int maxX = 0;
      int minY = 0;
      int maxY = 0;
      bool hasCalibration = false;
      uint64_t lastReadTick = 0;
      std::array<SlotState, c_maxTouchPoints> slots;
      TouchGestureFeed feed { c_maxTouchPoints };
    };

    struct TouchInput
    {
      TouchGestureIndevData common;
      MultiTouchState *state = nullptr;
    };

    uint64_t nowMs()
    {
      const auto now = std::chrono::steady_clock::now().time_since_epoch();
      return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    }

    int calibrate(int value, int inMin, int inMax, int outMin, int outMax)
    {
      int output = outMin;
      if(inMin != inMax)
      {
        output = (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
      }
      return std::clamp(output, outMin, outMax);
    }

    lv_point_t translatePoint(const MultiTouchState &state, const SlotState &slotState, int offsetX, int offsetY, int width, int height)
    {
      auto x = slotState.rawX;
      auto y = slotState.rawY;

      if(state.hasCalibration)
      {
        x = calibrate(slotState.rawX, state.minX, state.maxX, offsetX, offsetX + width - 1);
        y = calibrate(slotState.rawY, state.minY, state.maxY, offsetY, offsetY + height - 1);
      }

      return { x, y };
    }

    void updateSlots(MultiTouchState &state)
    {
      const auto tick = nowMs();
      if(state.lastReadTick == tick)
      {
        return;
      }

      state.lastReadTick = tick;

      input_event event {};
      while(read(state.fd, &event, sizeof(event)) > 0)
      {
        if(event.type == EV_ABS)
        {
          if(event.code == ABS_MT_SLOT)
          {
            state.currentSlot = std::clamp<int>(event.value, 0, c_maxTouchPoints - 1);
          }
          else if(event.code == ABS_MT_TRACKING_ID)
          {
            state.slots[state.currentSlot].active = event.value >= 0;
          }
          else if(event.code == ABS_MT_POSITION_X)
          {
            state.slots[state.currentSlot].rawX = event.value;
          }
          else if(event.code == ABS_MT_POSITION_Y)
          {
            state.slots[state.currentSlot].rawY = event.value;
          }
          else if(event.code == ABS_X)
          {
            state.slots[0].rawX = event.value;
          }
          else if(event.code == ABS_Y)
          {
            state.slots[0].rawY = event.value;
          }
        }
        else if(event.type == EV_KEY && event.code == BTN_TOUCH)
        {
          state.slots[0].active = event.value != 0;
        }
      }
    }

    void readTouch(lv_indev_t *indev, lv_indev_data_t *data)
    {
      auto *common = static_cast<TouchGestureIndevData *>(lv_indev_get_driver_data(indev));
      auto *touch = common ? static_cast<TouchInput *>(common->context) : nullptr;
      if(!touch || !touch->state || touch->state->fd < 0 || !touch->state->display)
      {
        data->state = LV_INDEV_STATE_RELEASED;
        data->point = lv_point_t { 0, 0 };
        return;
      }

      auto &state = *touch->state;
      const auto lvTick = lv_tick_get();
      const auto refreshNeeded = state.feed.shouldRefresh(lvTick);
      if(refreshNeeded)
      {
        updateSlots(state);
      }

      const auto offsetX = lv_display_get_offset_x(state.display);
      const auto offsetY = lv_display_get_offset_y(state.display);
      const auto width = lv_display_get_horizontal_resolution(state.display);
      const auto height = lv_display_get_vertical_resolution(state.display);
      if(refreshNeeded)
      {
        std::vector<TouchSample> touches;
        touches.reserve(c_maxTouchPoints);

        for(int i = 0; i < c_maxTouchPoints; i++)
        {
          if(const auto &slotState = state.slots[i]; slotState.active)
          {
            touches.push_back(TouchSample {
                .id = static_cast<uint64_t>(i),
                .point = translatePoint(state, slotState, offsetX, offsetY, width, height),
            });
          }
        }

        state.feed.update(touches, static_cast<uint32_t>(state.lastReadTick));
        state.feed.markRefreshed(lvTick);
      }

      applyTouchGestureFeed(indev, data, state.feed, common->lastPoint);
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

    lv_indev_t *createTouchIndev(MultiTouchState *state)
    {
      auto *touch = static_cast<TouchInput *>(lv_malloc_zeroed(sizeof(TouchInput)));
      if(!touch)
      {
        return nullptr;
      }

      touch->common.feed = &state->feed;
      touch->common.context = touch;
      touch->state = state;

      auto *indev = lv_indev_create();
      if(!indev)
      {
        lv_free(touch);
        return nullptr;
      }

      lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
      lv_indev_set_read_cb(indev, readTouch);
      lv_indev_set_driver_data(indev, &touch->common);
      lv_indev_set_display(indev, state->display);
      lv_indev_set_pinch_up_threshold(indev, 1.05f);
      lv_indev_set_pinch_down_threshold(indev, 0.95f);
      lv_indev_add_event_cb(indev, releaseTouch, LV_EVENT_DELETE, indev);
      return indev;
    }
  }

  Window::Window(Rect position, Rotation rotation)
  {
    lv_disp_t *disp = lv_linux_fbdev_create();
    lv_linux_fbdev_set_file(disp, "/dev/fb0");

    lv_display_set_offset(disp, position.pos.x, position.pos.y);
    lv_display_set_resolution(disp, position.size.w, position.size.h);
    lv_display_set_rotation(disp, static_cast<lv_display_rotation_t>(rotation));

    m_display = disp;

    auto *mtState = new MultiTouchState {};
    mtState->display = disp;

    const auto touchPath = discoverTouchEvdevPath();
    const char *touchDevice = touchPath ? touchPath->c_str() : nullptr;

    if(touchDevice)
    {
      mtState->fd = open(touchDevice, O_RDONLY | O_NONBLOCK);
    }

    if(mtState->fd >= 0)
    {
      input_absinfo absX {};
      input_absinfo absY {};
      if(ioctl(mtState->fd, EVIOCGABS(ABS_MT_POSITION_X), &absX) == 0 && ioctl(mtState->fd, EVIOCGABS(ABS_MT_POSITION_Y), &absY) == 0)
      {
        mtState->minX = absX.minimum;
        mtState->maxX = absX.maximum;
        mtState->minY = absY.minimum;
        mtState->maxY = absY.maximum;
        mtState->hasCalibration = true;
      }
      else if(ioctl(mtState->fd, EVIOCGABS(ABS_X), &absX) == 0 && ioctl(mtState->fd, EVIOCGABS(ABS_Y), &absY) == 0)
      {
        mtState->minX = absX.minimum;
        mtState->maxX = absX.maximum;
        mtState->minY = absY.minimum;
        mtState->maxY = absY.maximum;
        mtState->hasCalibration = true;
      }

      if(auto *touchIndev = createTouchIndev(mtState))
      {
        m_touchIndevs.push_back(touchIndev);
      }

      if(m_touchIndevs.empty() && touchDevice)
      {
        m_mouse = lv_evdev_create(LV_INDEV_TYPE_POINTER, touchDevice);
        lv_indev_set_display(m_mouse, disp);
      }
    }
    else if(touchDevice)
    {
      m_mouse = lv_evdev_create(LV_INDEV_TYPE_POINTER, touchDevice);
      lv_indev_set_display(m_mouse, disp);
    }

    m_backendCleanup = [mtState]
    {
      if(mtState->fd >= 0)
      {
        close(mtState->fd);
      }
      delete mtState;
    };
  }
}
