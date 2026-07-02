#pragma once
#include "CustomDrawingElement.h"
#include "DrawContext.h"

#include <compose/state/LabelData.h>
#include "compose/FreeTypeFont.h"
#include <functional>
#include <string>
#include <compose/TextWrap.h>

namespace Compose
{
  class MultiLineLabel : public Widget
  {
   public:
    using Widget::setModifier;
    using Widget::Widget;

    using AutorunStringCB = std::function<std::string()>;

    template <typename... tArgs>
    explicit MultiLineLabel(BaseWidget &parent, tArgs &&... args)
        : Widget(lv_canvas_create(parent.getHandle()))
    {
      setLabelRenderingFunction();
      setModifier(Text { "" });
      applyDefaultStyle(BaseWidget::getHandle());
      MultiLineLabel::setModifier(BackgroundColor { Color::TRANSPARENT() });
      MultiLineLabel::setModifier(PrimaryColor { Color::WHITE() });
      setModifier(VerticalAlign::TOP());
      setModifier(TextAlign::LEFT());
      setModifier(SizePercentage::FULL());

      doAutorun([handle = getHandle()] {
        const MultiLineLabel l(handle);
        const auto &labelData = l.getDataForKey<LabelData>(c_labelData);

        const auto [height] = labelData.height.get();
        const auto [width] = labelData.width.get();
        const auto &fontDesc = labelData.font.get();
        const auto &font = s_fontStorage->getFont(fontDesc);
        const auto text = labelData.text.get();

        auto wrapWidth = width;
        if(wrapWidth <= 0 || wrapWidth == LV_SIZE_CONTENT)
        {
          wrapWidth = lv_obj_get_width(handle);
        }

        if(wrapWidth > 0)
        {
          auto wrappedLines
              = Compose::text::wrapText(text.text, wrapWidth, [&font](auto t) { return font.getStringWidth(t); });

          if(height == LV_SIZE_CONTENT)
          {
            const auto lineCount = wrappedLines.size();
            const auto lineHeight = font.getFontHeight();
            const auto totalHeight = static_cast<int>(lineCount * lineHeight);

            lv_obj_set_height(handle, totalHeight);
          }

          if(width == LV_SIZE_CONTENT)
          {
            auto maxW = 0;
            for(const auto &line : wrappedLines)
            {
              maxW = std::max(maxW, font.getStringWidth(line));
            }
            lv_obj_set_width(handle, maxW);
          }
        }
      });
      (setModifier(std::forward<tArgs>(args)), ...);
    }

    void setModifier(Width w) const override;
    void setModifier(Height h) const override;

    void operator<<(AutorunStringCB &&cb) const;

    void setModifier(Text s) const;
    void setModifier(TextAlign a) const;
    void setModifier(VerticalAlign v) const;

    void setModifier(PrimaryColor s) const override;
    void setModifier(BackgroundColor c) const override;
    virtual void setModifier(Font s) const;

   private:
    void setLabelRenderingFunction() const;
    void setDrawCall(CustomDrawingElement::tDrawCB &&draw) const;
  };
}

#define MULTI_LINE_LABEL(...)                                                                                          \
  it.add(Compose::MultiLineLabel(it __VA_OPT__(, __VA_ARGS__))) << [=](Compose::MultiLineLabel && it)
