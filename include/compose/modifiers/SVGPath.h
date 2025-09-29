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

    bool operator==(const SVGPath &) const = default;
  };

  struct SVGFileContent
  {
    std::string content;
    bool operator==(const SVGFileContent &) const = default;
  };
}