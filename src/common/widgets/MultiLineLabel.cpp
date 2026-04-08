#include <compose/widgets/MultiLineLabel.h>
#include <compose/widgets/DrawContext.h>
#include <compose/widgets/LabelShared.h>
#include <compose/FreeTypeFont.h>

#include <algorithm>
#include <utility>
#include <sstream>
#include <vector>

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
              = nltools::text::wrapText(displayText, w, [&](auto &text) { return font.getStringWidth(text); });
          const auto lineHeight = font.getFontHeight();
          auto boundsTop = 0;
          auto boundsBottom = 0;
          auto hasBounds = false;
          for(size_t i = 0; i < lines.size(); i++)
          {
            const auto lineBounds = font.getTextBounds(lines[i]);
            const auto lineTop = static_cast<int>(i) * lineHeight + lineBounds.top;
            const auto lineBottom = static_cast<int>(i) * lineHeight + lineBounds.bottom;

            if(!hasBounds)
            {
              boundsTop = lineTop;
              boundsBottom = lineBottom;
              hasBounds = true;
            }
            else
            {
              boundsTop = std::min(boundsTop, lineTop);
              boundsBottom = std::max(boundsBottom, lineBottom);
            }
          }
          const auto blockHeight = std::max(boundsBottom - boundsTop, 0);
          int startY = 0;
          switch(vertAlign.it)
          {
            case VerticalAlign::Top:
              startY = 0;
              break;
            case VerticalAlign::Bottom:
              startY = h - blockHeight;
              break;
            default:
            case VerticalAlign::Center:
              startY = (h - blockHeight) / 2;
              break;
          }

          const auto baseColor = cd.primaryColor.get();
          for(size_t i = 0; i < lines.size(); i++)
          {
            const auto &line = lines[i];
            const auto textWidth = font.getStringWidth(line);
            const auto startX = LabelShared::computeStartX(w, textWidth, textAlign);
            const auto y = startY - boundsTop + static_cast<int>(i) * lineHeight;

            ctx.drawText(line, startX, y, font, baseColor);
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
    nltools_detailedAssertAlways(w.it != LV_SIZE_CONTENT, "Width::FIT_CONTENT not allowed for MultiLineLabels");
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
