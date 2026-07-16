#pragma once
#include "src/draw/lv_draw_buf.h"
#include <compose/widgets/Widget.h>

#include <functional>
#include <memory>

namespace Compose
{
  class DrawContext;

  // A bitmap rendered once via renderBitmap(). Own it (e.g. in application-side
  // caches) to reuse the pixels across widget rebuilds - BITMAP_CACHE elements
  // just blit it.
  struct CachedBitmap
  {
    CachedBitmap(lv_draw_buf_t* buffer, int width, int height);
    ~CachedBitmap();
    CachedBitmap(const CachedBitmap&) = delete;
    CachedBitmap& operator=(const CachedBitmap&) = delete;

    lv_draw_buf_t* buffer;
    int width;
    int height;
  };

  using CachedBitmapPtr = std::shared_ptr<const CachedBitmap>;

  CachedBitmapPtr renderBitmap(int width, int height, const std::function<void(DrawContext&, int, int)>& draw);

  class BitmapCacheElement : public Widget
  {
   public:
    using Widget::setModifier;
    using Widget::Widget;

    // Reactive callback handing back the bitmap to show for the given size.
    // Returning null keeps the last shown bitmap.
    using tBitmapCB = std::function<CachedBitmapPtr(int, int)>;

    explicit BitmapCacheElement(WidgetType* w)
        : Widget(w)
    {
    }

    explicit BitmapCacheElement(Widget& parent);

    template <typename... tArgs>
    explicit BitmapCacheElement(Widget& parent, tArgs&&... args)
        : BitmapCacheElement(parent)
    {
      setModifiers(this, parent, std::forward<tArgs>(args)...);
    }

    void setBitmapCall(tBitmapCB&& cb) const;

    struct
    {
      void operator<<(tBitmapCB&& cb) const
      {
        m_parent->setBitmapCall(std::move(cb));
      }

      BitmapCacheElement* m_parent;
    } bitmap { this };
  };
}

#define BITMAP_CACHE(...) it.add(Compose::BitmapCacheElement(it __VA_OPT__(, __VA_ARGS__))) << [=](Compose::BitmapCacheElement && it)
#define BITMAP it.bitmap << [=]
