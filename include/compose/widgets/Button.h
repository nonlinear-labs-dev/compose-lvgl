#pragma once
#include "Widget.h"
#include "handler/Clicks.h"
#include <functional>
#include <string>
#include <gtkmm.h>

namespace Compose
{
  class Button : public Widget<Gtk::Button>
  {
   public:
    using AutorunStringCB = std::function<std::string()>;
    using Widget::setModifier;

    template <typename... tArgs>
    explicit Button(tArgs... args)
        : Widget(Gtk::make_managed<Gtk::Button>())
    {
      (setModifier(args), ...);
    }

    explicit Button(WidgetType* handle);
    void operator<<(AutorunStringCB&& cb) const;

    void setModifier(Text t) const;
    void setModifier(LabelCrop c) const;

   private:
    Gtk::Label* getLabel() const;
    mutable sigc::connection m_onClickConnection;
  };
}

#define BUTTON(...) it.add(Compose::Button(__VA_ARGS__)) << [=](Compose::Button&& it)
