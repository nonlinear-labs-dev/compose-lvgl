#pragma once
#include <gtkmm/enums.h>
#include <nltools/enums/EnumDecl.h>

ENUM(Alignment, int, END = Gtk::Align::ALIGN_END, START = Gtk::Align::ALIGN_START, FILL = Gtk::Align::ALIGN_FILL,
     CENTER = Gtk::Align::ALIGN_CENTER, BASELINE = Gtk::Align::ALIGN_BASELINE);

namespace Compose
{
  template <typename T> struct Align
  {
    Alignment it;
    constexpr static T END()
    {
      return { Alignment::END };
    }
    constexpr static T START()
    {
      return { Alignment::START };
    }
    constexpr static T CENTER()
    {
      return { Alignment::CENTER };
    }
    constexpr static T BASELINE()
    {
      return { Alignment::BASELINE };
    }
    constexpr static T FILL()
    {
      return { Alignment::FILL };
    }
  };

  struct HAlign : Align<HAlign>
  {
  };

  struct VAlign : Align<VAlign>
  {
  };
}