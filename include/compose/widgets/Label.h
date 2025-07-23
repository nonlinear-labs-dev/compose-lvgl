#pragma once
#include "Widget.h"
#include "src/widgets/label/lv_label.h"

#include <functional>
#include <string>

namespace Compose
{
  class Label final : public Widget
  {
   public:
    using Widget::setModifier;

    using AutorunStringCB = std::function<std::string()>;

    template <typename... tArgs>
    explicit Label(BaseWidget& parent, tArgs... args)
        : Widget(lv_label_create(parent.getHandle()))
    {
      applyDefaultStyle(BaseWidget::getHandle());
      setModifier(BackgroundColor { Color::TRANSPARENT() });
      (setModifier(args), ...);
    }

    explicit Label(WidgetType* handle);
    void operator<<(AutorunStringCB&& cb) const;

    struct TextAlign
    {
      lv_text_align_t it;
    };

    void setModifier(TextAlign a) const
    {
      lv_obj_set_style_text_align(getHandle(), a.it, LV_PART_MAIN);
    }

    void setModifier(Text s) const;
    void setModifier(PrimaryColor s) const override;
    void setModifier(FontSize s) const;
  };
}

#define LABEL(...) it.add(Compose::Label(it __VA_OPT__(, __VA_ARGS__))) << [=](Compose::Label&& it)
