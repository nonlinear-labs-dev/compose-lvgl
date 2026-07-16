#include "compose/widgets/BitmapCache.h"
#include "compose/widgets/DrawContext.h"

#include "reactive/Computation.h"
#include "reactive/Deferrer.h"

#include "src/core/lv_obj.h"
#include "src/misc/cache/instance/lv_image_cache.h"
#include "src/widgets/canvas/lv_canvas.h"
#include "src/widgets/image/lv_image.h"

namespace Compose
{
  CachedBitmap::CachedBitmap(lv_draw_buf_t* buffer, int width, int height)
      : buffer(buffer)
      , width(width)
      , height(height)
  {
  }

  CachedBitmap::~CachedBitmap()
  {
    lv_image_cache_drop(buffer);
    lv_draw_buf_destroy(buffer);
  }

  CachedBitmapPtr renderBitmap(int width, int height, const std::function<void(DrawContext&, int, int)>& draw)
  {
    auto buffer = lv_draw_buf_create(width, height, LV_COLOR_FORMAT_ARGB8888, LV_STRIDE_AUTO);
    lv_draw_buf_clear(buffer, nullptr);

    auto canvas = lv_canvas_create(lv_layer_top());
    lv_obj_add_flag(canvas, LV_OBJ_FLAG_HIDDEN);
    lv_canvas_set_draw_buf(canvas, buffer);

    {
      LVGLDrawContext context(*canvas);
      draw(context, width, height);
    }

    lv_obj_delete(canvas);

    return std::make_shared<CachedBitmap>(buffer, width, height);
  }

  namespace
  {
    struct BitmapData
    {
      explicit BitmapData(lv_obj_t* handle)
          : m_handle(handle)
      {
        m_size = { lv_obj_get_width(handle), lv_obj_get_height(handle) };

        m_resizeHandler = lv_obj_add_event_cb(
            handle,
            [](lv_event_t* e)
            {
              Reactive::Deferrer deferrer;
              const auto data = static_cast<BitmapData*>(lv_event_get_user_data(e));
              data->m_size = { lv_obj_get_width(data->m_handle), lv_obj_get_height(data->m_handle) };
            },
            LV_EVENT_SIZE_CHANGED, this);
      }

      ~BitmapData()
      {
        if(m_handle && lv_obj_is_valid(m_handle))
        {
          if(m_resizeHandler)
            lv_obj_remove_event_dsc(m_handle, m_resizeHandler);
        }
      }

      Reactive::Var<std::pair<int, int>> m_size { { 0, 0 } };
      CachedBitmapPtr m_shown;
      lv_event_dsc_t* m_resizeHandler = nullptr;
      lv_obj_t* m_handle = nullptr;
    };
  }

  BitmapCacheElement::BitmapCacheElement(Widget& parent)
      : Widget(lv_image_create(parent.getHandle()))
  {
    applyDefaultStyle(BaseWidget::getHandle());
  }

  void BitmapCacheElement::setBitmapCall(tBitmapCB&& cb) const
  {
    Widget(getHandle())
        .doAutorun(
            [cb = std::move(cb), handle = getHandle()]
            {
              const Widget widget(handle);

              auto& data
                  = widget.ensureDataForKeyExistsOwning<BitmapData>(c_bitmapData, [handle] { return new BitmapData(handle); });

              const auto [width, height] = data.m_size.get();

              if(width > 0 && height > 0)
              {
                CachedBitmapPtr bitmap;
                try
                {
                  bitmap = cb(width, height);
                }
                catch(std::exception&)
                {
                }

                if(bitmap && bitmap != data.m_shown)
                {
                  data.m_shown = bitmap;
                  lv_image_set_src(handle, bitmap->buffer);
                }
              }
            });
  }
}
