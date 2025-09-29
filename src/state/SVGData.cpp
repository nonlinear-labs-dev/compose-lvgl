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

  void SVGData::renderToDrawContext(DrawContext& ctx, int width, int height)
  {
    auto start = std::chrono::high_resolution_clock::now();

    auto doc = document.get().get();
    if(!doc)
      return;

    auto bitmap = doc->renderToBitmap(width, height);
    if(!bitmap.valid())
      return;

    const auto* data = bitmap.data();
    const auto stride = bitmap.stride();

    for(int y = 0; y < height; ++y)
    {
      for(int x = 0; x < width; ++x)
      {
        const auto* pixel = data + y * stride + x * 4;
        const auto r = pixel[2];
        const auto g = pixel[1];
        const auto b = pixel[0];
        const auto a = pixel[3];

        if(a > 0)
        {
          Color color;
          color.r = r;
          color.g = g;
          color.b = b;
          color.a = a / 255.0f;

          Point point { x, y };
          Rect rect { point, { 1, 1 } };
          ctx.fillRect(color, rect);
        }
      }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    nltools::Log::error("rendering took", diff.count(), "ms");
  }
}
