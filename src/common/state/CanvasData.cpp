#include "compose/state/CanvasData.h"
#include "compose/widgets/DrawContext.h"
#include "reactive/Computation.h"

namespace Compose
{
  CanvasData::CanvasData(lv_obj_t* handle, CustomDrawingElement::tDrawCB cb)
      : m_drawCallback(std::move(cb))
      , m_handle(handle)
  {
    ensureBuffer();

    m_resizeHandler = lv_obj_add_event_cb(
        handle,
        [](lv_event_t* e) {
          Reactive::Deferrer deferrer;
          const auto data = static_cast<CanvasData*>(lv_event_get_user_data(e));
          data->ensureBuffer();
        },
        LV_EVENT_SIZE_CHANGED, this);
  }

  void CanvasData::ensureBuffer()
  {
    Reactive::Deferrer deferrer;
    const int width = lv_obj_get_width(this->m_handle);
    const int height = lv_obj_get_height(this->m_handle);

    if(width != m_lastBufferWidth || height != m_lastBufferHeight)
    {
      m_lastBufferWidth = width;
      m_lastBufferHeight = height;
      auto b = lv_draw_buf_create(width, height, LV_COLOR_FORMAT_ARGB8888, LV_STRIDE_AUTO);
      lv_draw_buf_clear(b, nullptr);
      lv_canvas_set_buffer(this->m_handle, b->data, width, height, LV_COLOR_FORMAT_ARGB8888);
      m_buffer.modify([=](auto& f) { f.reset(b); });
    }
  }

  CanvasData::~CanvasData()
  {
    if(m_handle && lv_obj_is_valid(m_handle))
    {
      if(m_resizeHandler)
        lv_obj_remove_event_dsc(m_handle, m_resizeHandler);
    }
  }
}
