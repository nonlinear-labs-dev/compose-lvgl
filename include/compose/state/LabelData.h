#pragma once
#include "CanvasData.h"

namespace Compose
{
  struct LabelData : CanvasData
  {
    using CanvasData::CanvasData;

    Reactive::Var<BackgroundColor> bgColor;
    Reactive::Var<Font> font;
    Reactive::Var<Text> text;
    Reactive::Var<TextAlign> align;
    Reactive::Var<VerticalAlign> verticalAlign = VerticalAlign::CENTER();
    Reactive::Var<PrimaryColor> primaryColor;
    Reactive::Var<Width> width;
    Reactive::Var<Height> height;
  };
}