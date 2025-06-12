#pragma once
#include <filesystem>
#include <optional>
#include "Size.h"

namespace Compose
{
  struct SVGPath
  {
    std::filesystem::path it;
    std::optional<Size> size = std::nullopt;
  };
} 