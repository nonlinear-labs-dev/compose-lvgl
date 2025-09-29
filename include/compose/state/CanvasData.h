#pragma once
#include <compose/modifiers/Modifiers.h>
#include <memory>
#include <reactive/Var.h>
#include <compose/widgets/CustomDrawingElement.h>

namespace Compose
{
  struct CanvasData
  {
    using tBufferPtr = std::unique_ptr<lv_draw_buf_t, decltype(&lv_draw_buf_destroy)>;

    Reactive::Var<tBufferPtr> buffer { { nullptr, lv_draw_buf_destroy } };
    CustomDrawingElement::tDrawCB drawCallback;
    lv_event_dsc_t* resizeHandler = nullptr;
    lv_obj_t* handle = nullptr;

    CanvasData(lv_obj_t* handle, CustomDrawingElement::tDrawCB cb);
    ~CanvasData();
    void ensureBuffer();
  };

  struct LabelData : CanvasData
  {
    using CanvasData::CanvasData;

    Reactive::Var<BackgroundColor> bgColor;
    Reactive::Var<Font> font;
    Reactive::Var<Text> text;
    Reactive::Var<TextAlign> align;
    Reactive::Var<VerticalAlign> verticalAlign;
    Reactive::Var<PrimaryColor> primaryColor;
  };
}