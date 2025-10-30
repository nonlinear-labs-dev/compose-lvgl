#include "compose/state/CanvasData.h"

#include <compose/widgets/Label.h>
#include <compose/widgets/DrawContext.h>
#include <compose/widgets/LabelShared.h>
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

          const auto startX = LabelShared::computeStartX(w, textWidth, textAlign);

          const auto startY = LabelShared::computeStartYSingle(h, font, displayText, vertAlign);

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
    LabelShared::setText(*this, t);
  }

  void Label::setModifier(PrimaryColor c) const
  {
    LabelShared::setPrimaryColor(*this, c);
  }

  void Label::setModifier(BackgroundColor c) const
  {
    LabelShared::setBackgroundColor(*this, c);
  }

  void Label::setModifier(Font s) const
  {
    LabelShared::setFont(*this, s);
  }

  void Label::setModifier(TextAlign a) const
  {
    LabelShared::setTextAlign(*this, a);
  }

  void Label::setModifier(VerticalAlign v) const
  {
    LabelShared::setVerticalAlign(*this, v);
  }

  void Label::setDrawCall(CustomDrawingElement::tDrawCB &&draw) const
  {
    LabelShared::setDrawCallCommon(*this, std::move(draw));
  }

  void Label::setModifier(Width w) const
  {
    LabelShared::setWidth(*this, w);
  }

  void Label::setModifier(Height h) const
  {
    LabelShared::setHeight(*this, h);
  }

  void Label::operator<<(AutorunStringCB &&cb) const
  {
    doAutorun([cb = std::move(cb), label = getHandle()] { Label(label).setModifier(Text { cb() }); });
  }

}