#pragma once
#include "Widget.h"
#include "src/widgets/label/lv_label.h"

#include <functional>
#include <string>

namespace Compose
{
  class Label final : public BaseWidget
  {
   public:
    using AutorunStringCB = std::function<std::string()>;

    template <typename... tArgs>
    explicit Label(tArgs... args)
        : BaseWidget(lv_label_create(lv_screen_active()))
    {
      (setModifier(args), ...);
    }

    explicit Label(WidgetType* handle);
    void operator<<(AutorunStringCB&& cb) const;

    // void setModifier(Text s) const;
    // void setModifier(PrimaryColor s) const override;
    // void setModifier(FontSize s) const;
    // void setModifier(FontWeight w) const;
    // void setModifier(XAlign x) const;
    // void setModifier(LabelCrop c) const;
    // void setModifier(Justify j) const;
    // void setModifier(LetterSpacing j) const;
    // void setModifier(MonoSpaceNumbers m) const;
  };
}

#define LABEL(...) it.add(Compose::Label(__VA_ARGS__)) << [=](Compose::Label&& it)
