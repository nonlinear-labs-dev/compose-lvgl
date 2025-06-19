#include "compose/widgets/DrawContext.h"
#include <compose/widgets/CustomDrawingElement.h>
#include "src/widgets/canvas/lv_canvas.h"
#include "src/draw/lv_draw_buf.h"
#include "src/misc/lv_color.h"

namespace Compose
{
  struct CanvasBufferData
  {
    std::shared_ptr<lv_draw_buf_t> buffer;
    CustomDrawingElement::tDrawCB drawCallback;

    explicit CanvasBufferData(CustomDrawingElement::tDrawCB cb)
        : buffer(nullptr)
        , drawCallback(std::move(cb))
    {
    }
  };

  struct SharedPtrWrapper
  {
    std::shared_ptr<CanvasBufferData> data_ptr;

    explicit SharedPtrWrapper(std::shared_ptr<CanvasBufferData> data)
        : data_ptr(std::move(data))
    {
    }
  };

  constexpr static auto CANVAS_DATA_KEY = "_CanvasDataKey";

  auto createNewBuffer(int w, int h)
  {
    auto rawBuffer = lv_draw_buf_create(w, h, LV_COLOR_FORMAT_ARGB8888, LV_STRIDE_AUTO);
    return std::shared_ptr<lv_draw_buf_t>(rawBuffer,
                                          [](lv_draw_buf_t* buf)
                                          {
                                            if(buf)
                                              lv_draw_buf_destroy(buf);
                                          });
  }

  static void handleResizeEvent(lv_event_t* e)
  {
    if(lv_event_get_code(e) == LV_EVENT_SIZE_CHANGED)
    {
      const auto wrapper = static_cast<SharedPtrWrapper*>(lv_event_get_user_data(e));
      if(wrapper && wrapper->data_ptr)
      {
        const auto handle = static_cast<lv_obj_t*>(lv_event_get_target(e));
        const int width = lv_obj_get_width(handle);
        const int height = lv_obj_get_height(handle);

        if(width > 0 && height > 0)
        {
          const auto newBuffer = createNewBuffer(width, height);

          if(newBuffer != nullptr)
          {
            wrapper->data_ptr->buffer = newBuffer;
            lv_canvas_set_draw_buf(handle, newBuffer.get());
            LVGLDrawContext drawContext(handle);
            wrapper->data_ptr->drawCallback(drawContext, width, height);
          }
        }
      }
    }
  }

  void CustomDrawingElement::setDrawCall(tDrawCB&& draw) const
  {
    auto handle = getHandle();
    if(!handle)
      return;

    auto bufferData = std::make_shared<CanvasBufferData>(std::move(draw));
    auto* wrapper = new SharedPtrWrapper(bufferData);

    ensureDataForKeyExistsNonOwning<SharedPtrWrapper>(CANVAS_DATA_KEY, [wrapper] { return wrapper; });

    lv_obj_add_event_cb(handle, &handleResizeEvent, LV_EVENT_SIZE_CHANGED, wrapper);
    lv_obj_add_event_cb(
        handle,
        [](lv_event_t* e)
        {
          if(lv_event_get_code(e) == LV_EVENT_DELETE)
          {
            delete static_cast<SharedPtrWrapper*>(lv_event_get_user_data(e));
          }
        },
        LV_EVENT_DELETE, wrapper);

    lv_obj_update_layout(handle);
    const auto width = lv_obj_get_width(handle);
    const auto height = lv_obj_get_height(handle);

    if(width <= 0 || height <= 0)
      return;

    const auto newBuffer = createNewBuffer(width, height);

    if(newBuffer == nullptr)
      return;

    bufferData->buffer = newBuffer;
    lv_canvas_set_draw_buf(handle, newBuffer.get());

    lv_obj_add_event_cb(
        handle,
        [](lv_event_t* e)
        {
          const auto handle = static_cast<lv_obj_t*>(lv_event_get_target(e));
          const auto wrapper = static_cast<SharedPtrWrapper*>(lv_event_get_user_data(e));
          lv_obj_update_layout(handle);
          if(wrapper && wrapper->data_ptr)
          {
            LVGLDrawContext drawContext(handle);
            wrapper->data_ptr->drawCallback(drawContext, lv_obj_get_width(handle), lv_obj_get_height(handle));
          }
        },
        LV_EVENT_READY, wrapper);

    doAutorun(
        [handle = getHandle(), wrapper]
        {
          LVGLDrawContext drawContext(handle);
          const auto w = lv_obj_get_width(handle);
          const auto h = lv_obj_get_height(handle);
          wrapper->data_ptr->drawCallback(drawContext, w, h);
        });
  }
}