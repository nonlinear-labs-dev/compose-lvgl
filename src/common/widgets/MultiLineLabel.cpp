#include <compose/widgets/MultiLineLabel.h>
#include <compose/widgets/DrawContext.h>
#include <compose/widgets/LabelShared.h>
#include <compose/FreeTypeFont.h>

#include <utility>
#include <sstream>
#include <vector>
#include <cassert>

namespace Compose
{

  void MultiLineLabel::setLabelRenderingFunction() const
  {
    setDrawCall(
        [handle = getHandle()](DrawContext &ctx, int w, int h)
        {
          const MultiLineLabel labelWidget(handle);
          const auto &cd = labelWidget.getDataForKey<LabelData>(c_labelData);

          if(cd.bgColor.get().a > 0)
            ctx.fillRect(cd.bgColor, { 0, 0, w, h });

          const auto &font = s_fontStorage->getFont(cd.font);
          const auto displayText = cd.text.get().text;
          const auto textAlign = cd.align.get();
          const auto vertAlign = cd.verticalAlign.get();

          const auto lines
              = Compose::text::wrapText(displayText, w, [&](auto &text) { return font.getStringWidth(text); });
          const auto lineHeight = font.getFontHeight();
          const auto totalTextHeight = static_cast<int>(lines.size()) * lineHeight;

          const auto startY = LabelShared::computeStartYBlock(h, totalTextHeight, lineHeight, font, vertAlign);

          const auto baseColor = cd.primaryColor.get();

          int y = startY;
          for(const auto &line : lines)
          {
            const auto textWidth = font.getStringWidth(line);
            const auto startX = LabelShared::computeStartX(w, textWidth, textAlign);

            ctx.drawText(line, startX, y, font, baseColor);

            y += lineHeight;
          }
        });
  }

  void MultiLineLabel::setModifier(Text t) const
  {
    LabelShared::setText(*this, t);
  }

  void MultiLineLabel::setModifier(PrimaryColor c) const
  {
    LabelShared::setPrimaryColor(*this, c);
  }

  void MultiLineLabel::setModifier(BackgroundColor c) const
  {
    LabelShared::setBackgroundColor(*this, c);
  }

  void MultiLineLabel::setModifier(Font s) const
  {
    LabelShared::setFont(*this, s);
  }

  void MultiLineLabel::setModifier(TextAlign a) const
  {
    LabelShared::setTextAlign(*this, a);
  }

  void MultiLineLabel::setModifier(VerticalAlign v) const
  {
    LabelShared::setVerticalAlign(*this, v);
  }

  void MultiLineLabel::setDrawCall(CustomDrawingElement::tDrawCB &&draw) const
  {
    LabelShared::setDrawCallCommon(*this, std::move(draw));
  }

  void MultiLineLabel::setModifier(Width w) const
  {
    assert(w.it != LV_SIZE_CONTENT && "Width::Fit_Content not allowed for Multi-LineLabels");
    LabelShared::setWidth(*this, w);
  }

  void MultiLineLabel::setModifier(Height h) const
  {
    LabelShared::setHeight(*this, h);
  }

  void MultiLineLabel::operator<<(AutorunStringCB &&cb) const
  {
    doAutorun([cb = std::move(cb), label = getHandle()] { MultiLineLabel(label).setModifier(Text { cb() }); });
  }
}
