#pragma once

#include "src/indev/lv_indev.h"

#include <cstddef>
#include <cstdint>

namespace Compose
{
  struct TouchIndevData
  {
    static constexpr uint32_t c_magic = 0x54494E44;

    uint32_t magic = c_magic;
    uint32_t pointerId = 0;
    const size_t *activeTouchCount = nullptr;
    void *context = nullptr;
  };

  inline const TouchIndevData *getTouchIndevData(const lv_indev_t *indev)
  {
    if(auto *touch = static_cast<const TouchIndevData *>(lv_indev_get_driver_data(indev)))
    {
      if(touch->magic == TouchIndevData::c_magic)
      {
        return touch;
      }
    }

    return nullptr;
  }

  inline uint32_t touchPointerId(const lv_indev_t *indev)
  {
    if(auto *touch = getTouchIndevData(indev))
    {
      return touch->pointerId;
    }

    return 0;
  }

  inline size_t activeTouchCount(const lv_indev_t *indev)
  {
    if(auto *touch = getTouchIndevData(indev))
    {
      if(touch->activeTouchCount)
      {
        return *touch->activeTouchCount;
      }
    }

    return 0;
  }
}
