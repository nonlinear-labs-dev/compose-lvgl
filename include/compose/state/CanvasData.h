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

    CanvasData(lv_obj_t* handle, CustomDrawingElement::tDrawCB cb);
    ~CanvasData();
    void ensureBuffer();

    Reactive::Var<tBufferPtr> m_buffer { { nullptr, lv_draw_buf_destroy } };
    CustomDrawingElement::tDrawCB m_drawCallback;
    lv_event_dsc_t* m_resizeHandler = nullptr;
    lv_obj_t* m_handle = nullptr;
    std::optional<int> m_lastBufferWidth;
    std::optional<int> m_lastBufferHeight;
  };
}