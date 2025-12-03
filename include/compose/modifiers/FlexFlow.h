#pragma once

enum class FlexFlowEnum : int
{
  HORIZONTAL,
  HORIZONTAL_REVERSE,
  HORIZONTAL_WRAP,
  HORIZONTAL_WRAP_REVERSE,
  VERTICAL,
  VERTICAL_REVERSE,
  VERTICAL_WRAP,
  VERTICAL_WRAP_REVERSE,
};

namespace Compose
{
  struct FlexFlow
  {
    FlexFlowEnum it;

    static FlexFlow HORIZONTAL()
    {
      return { FlexFlowEnum::HORIZONTAL };
    }
    static FlexFlow HORIZONTAL_REVERSE()
    {
      return { FlexFlowEnum::HORIZONTAL_REVERSE };
    }
    static FlexFlow HORIZONTAL_WRAP()
    {
      return { FlexFlowEnum::HORIZONTAL_WRAP };
    }
    static FlexFlow HORIZONTAL_WRAP_REVERSE()
    {
      return { FlexFlowEnum::HORIZONTAL_WRAP_REVERSE };
    }
    static FlexFlow VERTICAL()
    {
      return { FlexFlowEnum::VERTICAL };
    }
    static FlexFlow VERTICAL_REVERSE()
    {
      return { FlexFlowEnum::VERTICAL_REVERSE };
    }
    static FlexFlow VERTICAL_WRAP()
    {
      return { FlexFlowEnum::VERTICAL_WRAP };
    }
    static FlexFlow VERTICAL_WRAP_REVERSE()
    {
      return { FlexFlowEnum::VERTICAL_WRAP_REVERSE };
    }

    bool operator==(const FlexFlow &) const = default;
  };
}