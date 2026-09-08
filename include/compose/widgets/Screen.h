#pragma once
#include "compose/modifiers/Size.h"
#include <optional>
#include <string>

namespace Compose
{
  std::optional<Size> framebufferSize(const std::string& device);
}
