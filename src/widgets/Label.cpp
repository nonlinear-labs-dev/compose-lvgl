#include <compose/widgets/Label.h>
#include <compose/widgets/DrawContext.h>
#include <compose/FreeTypeFont.h>
#include <cassert>
#include <utility>

namespace Compose
{
  std::unique_ptr<FontStorage> s_fontStorage = nullptr;

  void Label::setRenderCallback() const
  {
    render << [handle = getHandle()](DrawContext &ctx, int w, int h)
    {
      const Label labelWidget(handle);
      ctx.fillRect(labelWidget.getModifier<BackgroundColor>(), { 0, 0, w, h });
      s_fontStorage->getFont(labelWidget.getModifier<Font>())
          .draw(labelWidget.getModifier<Text>().text, 0, 0,
                [&](auto x, auto y, auto value)
                {
                  auto factor = value / 255.0;
                  ctx.fillRect(labelWidget.getModifier<PrimaryColor>().multiply(factor), { x, y, 1, 1 });
                });
    };
  }

  Label::Label(WidgetType *handle)
      : Widget(handle)
  {
  }

  void Label::setModifier(const Text &t) const
  {
    ensureDataForKeyExistsOwning<Text>(typeid(t).name()) = t;
    lv_obj_invalidate(getHandle());
  }

  void Label::setModifier(PrimaryColor c) const
  {
    ensureDataForKeyExistsOwning<PrimaryColor>(typeid(c).name()) = c;
    lv_obj_invalidate(getHandle());
  }

  void Label::setModifier(BackgroundColor c) const
  {
    ensureDataForKeyExistsOwning<BackgroundColor>(typeid(c).name()) = c;
    lv_obj_invalidate(getHandle());
  }

  void Label::setModifier(Font s) const
  {
    ensureDataForKeyExistsOwning<Font>(typeid(Font).name()) = s;
    lv_obj_invalidate(getHandle());
  }

  void Label::setDrawCall(CustomDrawingElement::tDrawCB &&draw) const
  {
    doAutorun(
        [handle = getHandle(),
         canvasData = &ensureDataForKeyExistsOwning<CanvasData>(
             c_canvasData, [this, &draw] { return new CanvasData(getHandle(), std::move(draw)); })]
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
}
