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
        [](lv_event_t* e)
        {
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

    if(width <= 0 || height <= 0)
    {
      if(m_lastBufferWidth != width || m_lastBufferHeight != height)
      {
        m_lastBufferWidth = width;
        m_lastBufferHeight = height;
        setBuffer(1, 1);
      }
      return;
    }

    if(width != m_lastBufferWidth || height != m_lastBufferHeight)
    {
      m_lastBufferWidth = width;
      m_lastBufferHeight = height;
      setBuffer(width, height);

      Reactive::Computation::untracked(
          [=, this]
          {
            try
            {
              LVGLDrawContext drawContext(*(this->m_handle));
              this->m_drawCallback(drawContext, width, height);
            }
            catch(std::exception&)
            {
            }
          });
    }
  }

  // The canvas keeps pointing at whatever buffer it was given, so the old one
  // may only die once the new one is in place - a zero sized widget therefore
  // gets a minimal buffer rather than none at all.
  void CanvasData::setBuffer(int width, int height)
  {
    auto newBuffer = lv_draw_buf_create(width, height, LV_COLOR_FORMAT_ARGB8888, LV_STRIDE_AUTO);
    lv_draw_buf_clear(newBuffer, nullptr);
    lv_canvas_set_buffer(m_handle, newBuffer->data, width, height, LV_COLOR_FORMAT_ARGB8888);
    m_buffer.modify([=](auto& f) { f.reset(newBuffer); });
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
