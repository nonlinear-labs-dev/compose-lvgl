#include <compose/widgets/Label.h>
#include <compose/widgets/DrawContext.h>
#include <compose/FreeTypeFont.h>
#include <utility>

namespace Compose
{
  std::unique_ptr<FontStorage> s_fontStorage = nullptr;

  void Label::setRenderCallback() const
  {
    setDrawCall(
        [handle = getHandle()](DrawContext &ctx, int w, int h)
        {
          const Label labelWidget(handle);
          ctx.fillRect(labelWidget.getModifier<BackgroundColor>(), { 0, 0, w, h });

          const auto &font = s_fontStorage->getFont(labelWidget.getModifier<Font>());
          const auto displayText = labelWidget.getModifier<Text>().text;
          const auto textWidth = static_cast<int32_t>(font.getStringWidth(displayText));
          const auto textAlign = labelWidget.getModifier<TextAlign>();

          const auto startX = [w, textWidth](const TextAlign &a) -> int
          {
            switch(a.it)
            {
              case LV_TEXT_ALIGN_LEFT:
                return 0;
              case LV_TEXT_ALIGN_RIGHT:
                return w - textWidth;
              default:
              case LV_TEXT_ALIGN_CENTER:
              case LV_TEXT_ALIGN_AUTO:
                return (w - textWidth) / 2;
            }
          }(textAlign);

          const auto baseColor = labelWidget.getModifier<PrimaryColor>();

          font.draw(displayText, startX, 0,
                    [&](auto x, auto y, auto value)
                    {
                      auto factor = value / 255.0;
                      auto pixelColor = baseColor;
                      pixelColor.a = factor;
                      ctx.fillRect(pixelColor, { x, y, 1, 1 });
                    });
        });
  }

  void Label::setModifier(Text t) const
  {
    persistModifier(t);
    lv_obj_invalidate(getHandle());
  }

  void Label::setModifier(PrimaryColor c) const
  {
    persistModifier(c);
    lv_obj_invalidate(getHandle());
  }

  void Label::setModifier(BackgroundColor c) const
  {
    persistModifier(c);
    lv_obj_invalidate(getHandle());
  }

  void Label::setModifier(Font s) const
  {
    persistModifier(std::move(s));
    lv_obj_invalidate(getHandle());
  }

  void Label::setDrawCall(CustomDrawingElement::tDrawCB &&draw) const
  {
    doAutorun(
        [handle = getHandle(),
         canvasData = &ensureDataForKeyExistsOwning<CanvasData>(
             c_canvasData, [this, &draw] { return new CanvasData(getHandle(), std::move(draw)); })]
        {
          if(handle && canvasData && canvasData->drawCallback && lv_obj_is_valid(handle))
          {
            LVGLDrawContext drawContext(handle);
            const auto w = lv_obj_get_width(handle);
            const auto h = lv_obj_get_height(handle);
            canvasData->drawCallback(drawContext, w, h);
          }
        });
  }

  void Label::cleanup() const
  {
    auto storage = getUserDataStorage();
    if(storage)
    {
      auto it = storage->entries.find(c_canvasData);
      if(it != storage->entries.end() && it->second)
      {
        auto canvasData = static_cast<CanvasData *>(it->second->data);
        if(canvasData)
        {
          if(canvasData->handle && lv_obj_is_valid(canvasData->handle))
          {
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

  void Label::operator<<(AutorunStringCB &&cb) const
  {
    doAutorun([cb = std::move(cb), label = getHandle()] { Label(label).setModifier(Text { cb() }); });
  }

  void Label::setModifier(TextAlign a) const
  {
    persistModifier(a);
    lv_obj_invalidate(getHandle());
  }
}
