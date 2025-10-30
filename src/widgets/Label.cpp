#include "compose/state/CanvasData.h"

#include <compose/widgets/Label.h>
#include <compose/widgets/DrawContext.h>
#include <compose/FreeTypeFont.h>
#include <utility>

namespace Compose
{
  void Label::setLabelRenderingFunction() const
  {
    setDrawCall(
        [handle = getHandle()](DrawContext &ctx, int w, int h)
        {
          const Label labelWidget(handle);
          const auto &cd = labelWidget.getDataForKey<LabelData>(c_labelData);

          if(cd.bgColor.get().a > 0)
            ctx.fillRect(cd.bgColor, { 0, 0, w, h });

          const auto &font = s_fontStorage->getFont(cd.font);
          const auto displayText = cd.text.get().text;
          const auto textWidth = font.getStringWidth(displayText);
          const auto textAlign = cd.align.get();
          const auto vertAlign = cd.verticalAlign.get();

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

          const auto startY
              = [h, maxBottomOffset = font.getMaxBottomOffset(displayText), fontSize = font.getFontHeight(),
                 capHeightPx = font.getCapHeightPx()](const VerticalAlign &a) -> int
          {
            switch(a.it)
            {
              case VerticalAlign::Top:
                return 0;
              case VerticalAlign::Bottom:
                return h - maxBottomOffset;
              default:
              case VerticalAlign::Center:
                return h / 2 - fontSize + capHeightPx / 2;
            }
          }(vertAlign);

          const auto baseColor = cd.primaryColor.get();

          font.draw(displayText, startX, startY,
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
    getDataForKey<LabelData>(c_labelData).text = t;
  }

  void Label::setModifier(PrimaryColor c) const
  {
    getDataForKey<LabelData>(c_labelData).primaryColor = c;
  }

  void Label::setModifier(BackgroundColor c) const
  {
    getDataForKey<LabelData>(c_labelData).bgColor = c;
  }

  void Label::setModifier(Font s) const
  {
    getDataForKey<LabelData>(c_labelData).font = s;
  }

  void Label::setModifier(TextAlign a) const
  {
    getDataForKey<LabelData>(c_labelData).align = a;
  }

  void Label::setModifier(VerticalAlign v) const
  {
    getDataForKey<LabelData>(c_labelData).verticalAlign = v;
  }

  void Label::setDrawCall(CustomDrawingElement::tDrawCB &&draw) const
  {
    nltools_detailedAssertAlways(!doesDataForKeyExist<LabelData>(),
                                 "CanvasData should not exist, setting a new render callback is prohibited");
    Widget(getHandle())
        .doAutorun(
            [draw = std::move(draw), handle = getHandle()]
            {
              const Widget widget(handle);

              auto &canvasData = widget.ensureDataForKeyExistsOwning<LabelData>(c_labelData, [handle, d = draw]
                                                                                { return new LabelData(handle, d); });

              const auto w = lv_obj_get_width(handle);
              const auto h = lv_obj_get_height(handle);

              auto &bufferUser = canvasData.buffer.get();

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

  void Label::setModifier(Width w) const
  {
    getDataForKey<LabelData>(c_labelData).width = w;
    Widget::setModifier(w);
  }

  void Label::setModifier(Height h) const
  {
    getDataForKey<LabelData>(c_labelData).height = h;
    Widget::setModifier(h);
  }

  void Label::operator<<(AutorunStringCB &&cb) const
  {
    doAutorun([cb = std::move(cb), label = getHandle()] { Label(label).setModifier(Text { cb() }); });
  }

}