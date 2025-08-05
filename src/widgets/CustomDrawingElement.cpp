#include "compose/widgets/DrawContext.h"
#include <compose/widgets/CustomDrawingElement.h>
#include "src/widgets/canvas/lv_canvas.h"
#include "src/draw/lv_draw_buf.h"
#include "src/misc/lv_color.h"

namespace Compose
{
  struct CanvasData
  {
    lv_draw_buf_t* buffer = nullptr;
    CustomDrawingElement::tDrawCB drawCallback;
    lv_event_dsc_t* resizeHandler = nullptr;
    lv_event_dsc_t* readyHandler = nullptr;
    lv_event_dsc_t* deleteHandler = nullptr;

    CanvasData(lv_obj_t* handle, CustomDrawingElement::tDrawCB cb)
        : drawCallback(std::move(cb))
    {
      resizeHandler = lv_obj_add_event_cb(
          handle,
          [](lv_event_t* e)
          {
            if(lv_event_get_code(e) == LV_EVENT_SIZE_CHANGED)
            {
              const auto data = static_cast<CanvasData*>(lv_event_get_user_data(e));
              if(data)
              {
                const auto handle = static_cast<lv_obj_t*>(lv_event_get_target(e));
                const int width = lv_obj_get_width(handle);
                const int height = lv_obj_get_height(handle);

                if(width > 0 && height > 0)
                {
                  if(data->buffer)
                    lv_draw_buf_destroy(data->buffer);

                  data->buffer = lv_draw_buf_create(width, height, LV_COLOR_FORMAT_ARGB8888, LV_STRIDE_AUTO);
                  if(data->buffer)
                  {
                    lv_canvas_set_draw_buf(handle, data->buffer);
                    LVGLDrawContext drawContext(handle);
                    data->drawCallback(drawContext, width, height);
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
            lv_obj_update_layout(handle);
            if(data && data->drawCallback)
            {
              LVGLDrawContext drawContext(handle);
              data->drawCallback(drawContext, lv_obj_get_width(handle), lv_obj_get_height(handle));
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
                if(data->resizeHandler)
                  lv_obj_remove_event_dsc(static_cast<lv_obj_t*>(lv_event_get_target(e)), data->resizeHandler);
                if(data->readyHandler)
                  lv_obj_remove_event_dsc(static_cast<lv_obj_t*>(lv_event_get_target(e)), data->readyHandler);
                if(data->deleteHandler)
                  lv_obj_remove_event_dsc(static_cast<lv_obj_t*>(lv_event_get_target(e)), data->deleteHandler);
                if(data->buffer)
                  lv_draw_buf_destroy(data->buffer);
              }
            }
          },
          LV_EVENT_DELETE, this);
    }

    ~CanvasData()
    {
    }
  };

  static constexpr auto CANVAS_DATA_KEY = "CanvasData";

  void CustomDrawingElement::setDrawCall(tDrawCB&& draw) const
  {
    doAutorun(
        [handle = getHandle(),
         canvasData = &ensureDataForKeyExistsOwning<CanvasData>(
             CANVAS_DATA_KEY, [this, &draw] { return new CanvasData(getHandle(), std::move(draw)); })]
        {
          if(handle && canvasData && canvasData->drawCallback)
          {
            LVGLDrawContext drawContext(handle);
            const auto w = lv_obj_get_width(handle);
            const auto h = lv_obj_get_height(handle);
            canvasData->drawCallback(drawContext, w, h);
          }
        });
  }
}