#include "compose/state/CanvasData.h"
#include "compose/widgets/DrawContext.h"
#include "reactive/Computation.h"

namespace Compose
{
  CanvasData::CanvasData(lv_obj_t* handle, CustomDrawingElement::tDrawCB cb)
      : drawCallback(std::move(cb))
      , handle(handle)
  {
    ensureBuffer();

    resizeHandler = lv_obj_add_event_cb(
        handle,
        [](lv_event_t* e)
        {
          const auto data = static_cast<CanvasData*>(lv_event_get_user_data(e));
          data->ensureBuffer();
        },
        LV_EVENT_SIZE_CHANGED, this);
  }

  void CanvasData::ensureBuffer()
  {
    const int width = lv_obj_get_width(this->handle);
    const int height = lv_obj_get_height(this->handle);

    if(width > 0 && height > 0)
    {
      if(width == lastBufferWidth && height == lastBufferHeight)
      {
        return;
      }

      auto b = lv_draw_buf_create(width, height, LV_COLOR_FORMAT_ARGB8888, LV_STRIDE_AUTO);
      lv_draw_buf_clear(b, nullptr);

      buffer.modify([=, this](auto& f) {
        f.reset(b);
        lv_canvas_set_buffer(this->handle, b->data, width, height, LV_COLOR_FORMAT_ARGB8888);
        lastBufferWidth = width;
        lastBufferHeight = height;
      });

      try
      {
        Reactive::Computation::untracked([&] {
          LVGLDrawContext drawContext(*handle);
          drawCallback(drawContext, width, height);
        });
      }
      catch(...)
      {
      }
    }
  }

  CanvasData::~CanvasData()
  {
    if(handle && lv_obj_is_valid(handle))
    {
      if(resizeHandler)
        lv_obj_remove_event_dsc(handle, resizeHandler);
    }
  }
}
