#pragma once
#include "Widget.h"
#include "compose/modifiers/LetterSpacing.h"
#include "compose/modifiers/XAlign.h"
#include <functional>
#include <string>
#include <gtkmm/label.h>

namespace Compose
{
  class Label final : public Widget<Gtk::Label>
  {
   public:
    using AutorunStringCB = std::function<std::string()>;
    using Widget::setModifier;

    template <typename... tArgs>
    explicit Label(tArgs... args)
        : Widget(Gtk::make_managed<WidgetType>())
    {
      (setModifier(args), ...);
    }
    explicit Label(WidgetType* handle);

    void operator<<(AutorunStringCB&& cb) const;

    void setModifier(Text s) const;
    void setModifier(PrimaryColor s) const override;
    void setModifier(FontSize s) const;
    void setModifier(FontWeight w) const;
    void setModifier(XAlign x) const;
    void setModifier(LabelCrop c) const;
    void setModifier(Justify j) const;
    void setModifier(LetterSpacing j) const;
    void setModifier(MonoSpaceNumbers m) const;
  };
}

#define LABEL(...) it.add(Compose::Label(__VA_ARGS__)) << [=](Compose::Label&& it)
