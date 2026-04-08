#pragma once
#include "CustomDrawingElement.h"
#include "DrawContext.h"

#include <compose/state/LabelData.h>
#include "compose/FreeTypeFont.h"
#include <functional>
#include <string>

namespace Compose
{
  class Label : public Widget
  {
   public:
    using Widget::setModifier;
    using Widget::Widget;

    using AutorunStringCB = std::function<std::string()>;

    template <typename... tArgs>
    explicit Label(BaseWidget &parent, tArgs... args)
        : Widget(lv_canvas_create(parent.getHandle()))
    {
      setLabelRenderingFunction();
      setModifier(Text { "" });
      applyDefaultStyle(BaseWidget::getHandle());
      Label::setModifier(BackgroundColor { Color::TRANSPARENT() });
      Label::setModifier(PrimaryColor { Color::WHITE() });
      setModifier(VerticalAlign::CENTER());
      setModifier(TextAlign::CENTER());
      setModifier(SizePercentage::FULL());

      doAutorun(
          [handle = getHandle()]
          {
            const Label l(handle);
            const auto &labelData = l.getDataForKey<LabelData>(c_labelData);
            const auto [width] = labelData.width.get();
            const auto [height] = labelData.height.get();
            auto updateLayout = false;

            const auto &fontDesc = labelData.font.get();
            const auto &font = s_fontStorage->getFont(fontDesc);
            const auto text = labelData.text.get();

            if(width == LV_SIZE_CONTENT)
            {
              const auto textWidth = font.getStringWidth(text.text);
              lv_obj_set_width(handle, textWidth);
              updateLayout = true;
            }

            if(height == LV_SIZE_CONTENT)
            {
              const auto textBounds = font.getTextBounds(text.text);
              lv_obj_set_height(handle, textBounds.height());
              updateLayout = true;
            }

            if(updateLayout)
            {
              lv_obj_update_layout(handle);
            }
          });
      (setModifier(args), ...);
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

#define LABEL(...) it.add(Compose::Label(it __VA_OPT__(, __VA_ARGS__))) << [=](Compose::Label &&it)
#define USE_FONT_STORAGE(...) Compose::s_fontStorage = std::make_unique<Compose::FontStorage>(__VA_ARGS__);