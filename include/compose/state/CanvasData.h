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
    int lastBufferWidth = -1;
    int lastBufferHeight = -1;

    CanvasData(lv_obj_t* handle, CustomDrawingElement::tDrawCB cb);
    ~CanvasData();
    void ensureBuffer();
  };
}