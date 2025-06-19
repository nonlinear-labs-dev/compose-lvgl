#pragma once
#include <nltools/enums/EnumDecl.h>
#include <lvgl.h>

#define BUILD_ALIGN(name, value)                                                                                       \
  constexpr static Align name()                                                                                        \
  {                                                                                                                    \
    return { value };                                                                                                  \
  }

namespace Compose
{
  struct Align
  {
    lv_align_t it;

    [[deprecated("Use MIDDLE_RIGHT instead")]] constexpr static Align END()
    {
      return { LV_ALIGN_RIGHT_MID };
    }

    [[deprecated("Use MIDDLE_LEFT instead")]] constexpr static Align START()
    {
      return { LV_ALIGN_LEFT_MID };
    }

    BUILD_ALIGN(TOP_LEFT, LV_ALIGN_TOP_LEFT)
    BUILD_ALIGN(TOP_MID, LV_ALIGN_TOP_MID)
    BUILD_ALIGN(TOP_RIGHT, LV_ALIGN_TOP_RIGHT)
    BUILD_ALIGN(MIDDLE_LEFT, LV_ALIGN_LEFT_MID)
    BUILD_ALIGN(CENTER, LV_ALIGN_CENTER)
    BUILD_ALIGN(MIDDLE_RIGHT, LV_ALIGN_RIGHT_MID)
    BUILD_ALIGN(BOTTOM_LEFT, LV_ALIGN_BOTTOM_LEFT)
    BUILD_ALIGN(BOTTOM_MID, LV_ALIGN_BOTTOM_MID)
    BUILD_ALIGN(BOTTOM_RIGHT, LV_ALIGN_BOTTOM_RIGHT)
  };
}