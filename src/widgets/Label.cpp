#include <compose/widgets/Label.h>
#include <compose/widgets/DrawContext.h>
#include <compose/FreeTypeFont.h>
#include <utility>

namespace Compose
{
  std::unique_ptr<FontStorage> s_fontStorage = nullptr;

  void Label::setLabelRenderingFunction() const
  {
    setDrawCall(
        [handle = getHandle()](DrawContext &ctx, int w, int h)
        {
          // return;
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
    nltools_detailedAssertAlways(!doesDataForKeyExist<CanvasData>(),
                                 "CanvasData should not exist, setting a new render callback is prohibited");
    Widget(getHandle())
        .doAutorun(
            [draw = std::move(draw), handle = getHandle()]
            {
              const Widget widget(handle);

              auto &canvasData = widget.ensureDataForKeyExistsOwning<CanvasData>(c_canvasData, [handle, d = draw]
                                                                                 { return new CanvasData(handle, d); });

              const auto w = lv_obj_get_width(handle);
              const auto h = lv_obj_get_height(handle);

              //draw buffer is set and init
              LVGLDrawContext drawContext(handle);

              try
              {
                canvasData.drawCallback(drawContext, w, h);
              }
              catch(std::exception &)
              {
              }
            });
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
