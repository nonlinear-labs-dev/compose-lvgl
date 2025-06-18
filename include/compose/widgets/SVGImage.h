#pragma once
#include <compose/widgets/Widget.h>

namespace Compose
{
  // class SVGImage : public Widget<Gtk::Image>
  // {
  //  public:
  //   using Widget::setModifier;
  //   template <typename... tArgs>
  //   explicit SVGImage(tArgs... args)
  //       : Widget(Gtk::make_managed<Gtk::Image>())
  //   {
  //     (setModifier(args), ...);
  //   }
  //
  //   void setModifier(SVGPath p) const;
  //   void setModifier(PixelSize s) const;
  //   void setModifier(PrimaryColor col) const;
  // };
}

#define IMAGE(...) it.add(Compose::SVGImage(__VA_ARGS__));