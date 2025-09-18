#pragma once

enum class AttachOptionsEnum
{
  SHRINK,
  FILL,
  EXPAND
};

namespace Compose
{
  struct AttachOptions
  {
    AttachOptionsEnum it;
    static AttachOptions SHRINK()
    {
      return { AttachOptionsEnum::SHRINK };
    }

    static AttachOptions FILL()
    {
      return { AttachOptionsEnum::FILL };
    }
    static AttachOptions EXPAND()
    {
      return { AttachOptionsEnum::EXPAND };
    }

    bool operator==(const AttachOptions &) const = default;
  };
}