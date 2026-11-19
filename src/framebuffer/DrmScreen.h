#pragma once

#include "compose/widgets/Screen.h"
#include <lvgl.h>

#include <optional>
#include <string>

namespace Compose
{
  lv_display_t *createDrmDisplay(const std::string &card);
  std::optional<Size> drmScreenSize(const std::string &card);
}
