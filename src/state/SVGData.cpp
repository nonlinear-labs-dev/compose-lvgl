#include "compose/state/SVGData.h"
#include "compose/widgets/DrawContext.h"
#include "compose/modifiers/Color.h"
#include "compose/modifiers/Rect.h"

namespace Compose
{
  SVGData::SVGData(lv_obj_t* handle, CustomDrawingElement::tDrawCB cb)
      : drawCallback(std::move(cb))
      , handle(handle)
  {
    resizeHandler = lv_obj_add_event_cb(
        handle,
        [](lv_event_t* e)
        {
          const auto data = static_cast<SVGData*>(lv_event_get_user_data(e));
          // Trigger re-render when size changes
          if(data->document.get())
          {
            const auto w = lv_obj_get_width(data->handle);
            const auto h = lv_obj_get_height(data->handle);
            LVGLDrawContext drawContext(data->handle);
            try
            {
              data->drawCallback(drawContext, w, h);
            }
            catch(std::exception&)
            {
            }
          }
        },
        LV_EVENT_SIZE_CHANGED, this);
  }

  SVGData::~SVGData()
  {
    if(handle && lv_obj_is_valid(handle))
    {
      if(resizeHandler)
        lv_obj_remove_event_dsc(handle, resizeHandler);
    }
  }

  void SVGData::renderToDrawContext(DrawContext& ctx, int width, int height) const
  {
    const auto doc = document.get().get();
    if(!doc)
      return;

    const auto bitmap = doc->renderToBitmap(width, height);

    if(!bitmap.valid())
      return;

    const auto* data = bitmap.data();
    const auto stride = bitmap.stride();

    ctx.putBitmap({ width, height, stride, data }, { 0, 0 }, color.get());
  }
}
