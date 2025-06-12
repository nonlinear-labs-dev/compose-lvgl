#pragma once
#include <gtkmm/enums.h>
#include <nltools/enums/EnumDecl.h>

ENUM(OrientationEnum, int, HORIZONTAL = Gtk::Orientation::ORIENTATION_HORIZONTAL,
     VERTICAL = Gtk::Orientation::ORIENTATION_VERTICAL);

namespace Compose
{
  struct Orientation
  {
    OrientationEnum it;

    static Orientation HORIZONTAL()
    {
      return { OrientationEnum::HORIZONTAL };
    }
    static Orientation VERTICAL()
    {
      return { OrientationEnum::VERTICAL };
    }
  };
} 