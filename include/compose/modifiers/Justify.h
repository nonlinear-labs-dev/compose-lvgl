#pragma once
#include <gtkmm/enums.h>
#include <nltools/enums/EnumDecl.h>

ENUM(Justification, int, LEFT = Gtk::Justification::JUSTIFY_LEFT, RIGHT = Gtk::Justification::JUSTIFY_RIGHT,
     CENTER = Gtk::Justification::JUSTIFY_CENTER, FILL = Gtk::Justification::JUSTIFY_FILL);

namespace Compose
{
  struct Justify
  {
    Justification it;
    static Justify LEFT()
    {
      return { Justification::LEFT };
    }
    static Justify RIGHT()
    {
      return { Justification::RIGHT };
    }
    static Justify CENTER()
    {
      return { Justification::CENTER };
    }
    static Justify FILL()
    {
      return { Justification::FILL };
    }
  };
} 