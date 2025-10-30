#include <compose/widgets/MultiLineLabel.h>
#include <compose/widgets/DrawContext.h>
#include <compose/widgets/LabelShared.h>
#include <compose/FreeTypeFont.h>

#include <utility>
#include <sstream>
#include <vector>

namespace Compose
{
  static std::vector<std::string> wrapText(const FreeTypeFont& font, const std::string& text, int maxWidth)
  {
    std::vector<std::string> words;
    {
      std::stringstream ws(text);
      std::string w;
      while(ws >> w)
        words.push_back(w);
    }

    std::vector<std::string> lines;
    std::string currentLine;
    const std::string space = " ";

    for(const auto& word : words)
    {
      const auto candidate = currentLine.empty() ? word : currentLine + space + word;
      const auto width = font.getStringWidth(candidate);

      if(width <= maxWidth)
      {
        currentLine = candidate;
      }
      else
      {
        if(!currentLine.empty())
          lines.push_back(currentLine);

        if(font.getStringWidth(word) <= maxWidth)
        {
          currentLine = word;
        }
        else
        {
          std::string partial;
          for(char c : word)
          {
            std::string next = partial;
            next.push_back(c);
            if(font.getStringWidth(next) > maxWidth && !partial.empty())
            {
              lines.push_back(partial);
              partial.clear();
            }
            partial.push_back(c);
          }
          currentLine = partial;
        }
      }
    }

    if(!currentLine.empty())
      lines.push_back(currentLine);

    if(lines.empty())
      lines.emplace_back("");

    return lines;
  }

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

          const auto lines = wrapText(font, displayText, w);
          const auto lineHeight = font.getFontHeight();
          const auto totalTextHeight = static_cast<int>(lines.size()) * lineHeight;

          const auto startY = LabelShared::computeStartYBlock(h, totalTextHeight, lineHeight, font, vertAlign);

          const auto baseColor = cd.primaryColor.get();

          int y = startY;
          for(const auto& line : lines)
          {
            const auto textWidth = font.getStringWidth(line);
            const auto startX = LabelShared::computeStartX(w, textWidth, textAlign);

            font.draw(line, startX, y,
                      [&](auto x, auto yy, auto value)
                      {
                        auto factor = value / 255.0;
                        auto pixelColor = baseColor;
                        pixelColor.a = factor;
                        ctx.fillRect(pixelColor, { x, yy, 1, 1 });
                      });

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


