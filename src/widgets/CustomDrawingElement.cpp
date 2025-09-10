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
            if(canvasData->drawHandler)
            {
              lv_obj_remove_event_dsc(canvasData->handle, canvasData->drawHandler);
              canvasData->drawHandler = nullptr;
            }
            if(canvasData->resizeHandler)
            {
              lv_obj_remove_event_dsc(canvasData->handle, canvasData->resizeHandler);
              canvasData->resizeHandler = nullptr;
            }
            if(canvasData->readyHandler)
            {
              lv_obj_remove_event_dsc(canvasData->handle, canvasData->readyHandler);
              canvasData->readyHandler = nullptr;
            }
            if(canvasData->deleteHandler)
            {
              lv_obj_remove_event_dsc(canvasData->handle, canvasData->deleteHandler);
              canvasData->deleteHandler = nullptr;
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
    Widget(handle).doAutorun(
        [this]
        {
          auto counter = this->redrawCounter.get();

          if(!this->buffer)
          {
            const int width = lv_obj_get_width(this->handle);
            const int height = lv_obj_get_height(this->handle);

            if(width > 0 && height > 0)
            {
              this->buffer = lv_draw_buf_create(width, height, LV_COLOR_FORMAT_ARGB8888, LV_STRIDE_AUTO);
            }

            if(this->buffer)
            {
              lv_canvas_set_buffer(this->handle, this->buffer, width, height, LV_COLOR_FORMAT_ARGB8888);
            }
          }

          LVGLDrawContext drawContext(this->handle);
          const auto w = lv_obj_get_width(this->handle);
          const auto h = lv_obj_get_height(this->handle);
          try
          {
            this->drawCallback(drawContext, w, h);
          }
          catch(std::exception&)
          {
          }
        });

    resizeHandler = lv_obj_add_event_cb(
        handle,
        [](lv_event_t* e)
        {
          if(lv_event_get_code(e) == LV_EVENT_SIZE_CHANGED)
          {
            const auto data = static_cast<CanvasData*>(lv_event_get_user_data(e));
            const auto handle = static_cast<lv_obj_t*>(lv_event_get_target(e));

            if(data && handle && lv_obj_is_valid(handle) && data->resizeHandler)
            {
              const int width = lv_obj_get_width(handle);
              const int height = lv_obj_get_height(handle);

              if(width > 0 && height > 0)
              {
                if(data->buffer)
                  lv_draw_buf_destroy(data->buffer);

                data->buffer = lv_draw_buf_create(width, height, LV_COLOR_FORMAT_ARGB8888, LV_STRIDE_AUTO);
                if(data->buffer)
                {
                  data->redrawCounter = data->redrawCounter + 1;
                }
              }
            }
          }
        },
        LV_EVENT_SIZE_CHANGED, this);

    readyHandler = lv_obj_add_event_cb(
        handle,
        [](lv_event_t* e)
        {
          const auto handle = static_cast<lv_obj_t*>(lv_event_get_target(e));
          const auto data = static_cast<CanvasData*>(lv_event_get_user_data(e));

          if(data && handle && lv_obj_is_valid(handle))
          {
            if(data->drawCallback)
            {
              LVGLDrawContext drawContext(handle);
              try
              {
                data->drawCallback(drawContext, lv_obj_get_width(handle), lv_obj_get_height(handle));
              }
              catch(std::exception&)
              {
              }
            }
          }
        },
        LV_EVENT_READY, this);

    deleteHandler = lv_obj_add_event_cb(
        handle,
        [](lv_event_t* e)
        {
          if(lv_event_get_code(e) == LV_EVENT_DELETE)
          {
            const auto data = static_cast<CanvasData*>(lv_event_get_user_data(e));
            if(data)
            {
              if(data->buffer)
              {
                lv_draw_buf_destroy(data->buffer);
                data->buffer = nullptr;
              }
            }
          }
        },
        LV_EVENT_DELETE, this);
  }
  CanvasData::~CanvasData()
  {
    if(handle && lv_obj_is_valid(handle))
    {
      if(drawHandler)
        lv_obj_remove_event_dsc(handle, drawHandler);
      if(resizeHandler)
        lv_obj_remove_event_dsc(handle, resizeHandler);
      if(readyHandler)
        lv_obj_remove_event_dsc(handle, readyHandler);
      if(deleteHandler)
        lv_obj_remove_event_dsc(handle, deleteHandler);
      if(buffer)
        lv_draw_buf_destroy(buffer);
    }
    else if(buffer)
    {
      lv_draw_buf_destroy(buffer);
    }

    drawHandler = nullptr;
    resizeHandler = nullptr;
    readyHandler = nullptr;
    deleteHandler = nullptr;
    handle = nullptr;
  }
}