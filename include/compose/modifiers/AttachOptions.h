#pragma once
#include <gtkmm/enums.h>
#include <nltools/enums/EnumDecl.h>

ENUM(AttachOptionsEnum, int, EXPAND = Gtk::AttachOptions::EXPAND, SHRINK = Gtk::AttachOptions::SHRINK,
     FILL = Gtk::AttachOptions::FILL);

namespace Compose
{
  struct AttachOptions
  {
    AttachOptionsEnum it;
    static AttachOptions SHRINK()
    {
      return { AttachOptionsEnum::SHRINK };
    }

    static AttachOptions FILL()
    {
      return { AttachOptionsEnum::FILL };
    }
    static AttachOptions EXPAND()
    {
      return { AttachOptionsEnum::EXPAND };
    }
  };
} 