#pragma once
#include <nltools/enums/EnumDecl.h>

#define BUILD_ALIGN(name, value)                                                                                       \
  constexpr static lv_align_t name()                                                                                   \
  {                                                                                                                    \
    return { value };                                                                                                  \
  }

namespace Compose
{
  struct Align
  {
    lv_align_t it;

    [[deprecated("Use MIDDLE_RIGHT instead")]] constexpr static lv_align_t END()
    {
      return { lv_align_t::LV_ALIGN_RIGHT_MID };
    }

    [[deprecated("Use MIDDLE_LEFT instead")]] constexpr static lv_align_t START()
    {
      return { lv_align_t::LV_ALIGN_LEFT_MID };
    }

    BUILD_ALIGN(TOP_LEFT, lv_align_t::LV_ALIGN_TOP_LEFT)
    BUILD_ALIGN(TOP_MID, lv_align_t::LV_ALIGN_TOP_MID)
    BUILD_ALIGN(TOP_RIGHT, lv_align_t::LV_ALIGN_TOP_RIGHT)
    BUILD_ALIGN(MIDDLE_LEFT, lv_align_t::LV_ALIGN_LEFT_MID)
    BUILD_ALIGN(CENTER, lv_align_t::LV_ALIGN_CENTER)
    BUILD_ALIGN(MIDDLE_RIGHT, lv_align_t::LV_ALIGN_RIGHT_MID)
    BUILD_ALIGN(BOTTOM_LEFT, lv_align_t::LV_ALIGN_BOTTOM_LEFT)
    BUILD_ALIGN(BOTTOM_MID, lv_align_t::LV_ALIGN_BOTTOM_MID)
    BUILD_ALIGN(BOTTOM_RIGHT, lv_align_t::LV_ALIGN_BOTTOM_RIGHT)
  };
}