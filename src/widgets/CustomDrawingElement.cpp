#include "compose/widgets/DrawContext.h"
#include <compose/widgets/CustomDrawingElement.h>
#include "src/widgets/canvas/lv_canvas.h"
#include "src/draw/lv_draw_buf.h"
#include "src/misc/lv_color.h"

namespace Compose
{
  void CustomDrawingElement::setDrawCall(tDrawCB&& draw) const
  {
    auto& canvasData = ensureDataForKeyExistsOwning<CanvasData>(
        c_canvasData, [this, &draw] { return new CanvasData(getHandle(), std::move(draw)); });
    canvasData.drawCallback = draw;
    lv_obj_invalidate(getHandle());
  }

  void CustomDrawingElement::cleanup() const
  {
    if(const auto storage = getUserDataStorage())
    {
      if(const auto it = storage->entries.find(c_canvasData); it != storage->entries.end() && it->second)
      {
        if(const auto canvasData = static_cast<CanvasData*>(it->second->data))
        {
          if(canvasData->handle && lv_obj_is_valid(canvasData->handle))
          {
            if(canvasData->resizeHandler)
            {
              lv_obj_remove_event_dsc(canvasData->handle, canvasData->resizeHandler);
              canvasData->resizeHandler = nullptr;
            }
          }
          if(canvasData->buffer)
          {
            lv_draw_buf_destroy(canvasData->buffer);
            canvasData->buffer = nullptr;
          }
          canvasData->handle = nullptr;
        }
      }
    }
  }

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

    auto bufferCopy = buffer.peek();
    // if(bufferCopy && bufferCopy->header.w == width && bufferCopy->header.h == height)
    // {
    //   return;
    // }

    if(bufferCopy && bufferCopy->header.flags & LV_IMAGE_FLAGS_ALLOCATED)
    {
      lv_draw_buf_destroy(bufferCopy);
    }

    if(width > 0 && height > 0)
    {
      this->buffer = lv_draw_buf_create(width, height, LV_COLOR_FORMAT_ARGB8888, LV_STRIDE_AUTO);
    }

    if(this->buffer.peek())
    {
      lv_canvas_set_buffer(this->handle, this->buffer.peek()->data, width, height, LV_COLOR_FORMAT_ARGB8888);
    }
  }

  CanvasData::~CanvasData()
  {
    if(handle && lv_obj_is_valid(handle))
    {
      if(resizeHandler)
        lv_obj_remove_event_dsc(handle, resizeHandler);
      if(buffer)
        lv_draw_buf_destroy(buffer);
    }
    else if(buffer)
    {
      lv_draw_buf_destroy(buffer);
    }
  }
}