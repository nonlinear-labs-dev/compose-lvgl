#pragma once

namespace Compose
{
  struct TransformSize
  {
    int width;
    int height;

    bool operator==(const TransformSize &) const = default;
  };
}
