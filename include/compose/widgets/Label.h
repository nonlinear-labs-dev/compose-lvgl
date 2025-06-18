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
    using AutorunStringCB = std::function<std::string()>;

    template <typename... tArgs>
    explicit Label(BaseWidget& parent, tArgs... args)
        : Widget(lv_label_create(parent.getHandle()))
    {
      (setModifier(args), ...);
    }

    explicit Label(WidgetType* handle);
    void operator<<(AutorunStringCB&& cb) const;

    void setModifier(Text s) const;
    void setModifier(PrimaryColor s) const;
    // void setModifier(FontSize s) const;
    // void setModifier(FontWeight w) const;
    // void setModifier(XAlign x) const;
    // void setModifier(LabelCrop c) const;
    // void setModifier(Justify j) const;
    // void setModifier(LetterSpacing j) const;
    // void setModifier(MonoSpaceNumbers m) const;
  };
}

#define LABEL(...) it.add(Compose::Label(it __VA_OPT__(, __VA_ARGS__))) << [=](Compose::Label&& it)
