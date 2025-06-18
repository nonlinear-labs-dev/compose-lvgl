#include <compose/widgets/Button.h>
#include <compose/widgets/Label.h>
#include <reactive/Computations.h>

namespace Compose
{
  // Button::Button(WidgetType *handle)
  //     : Widget(handle)
  // {
  // }
  //
  // void Button::operator<<(AutorunStringCB &&cb) const
  // {
  //   doAutorun([cb = std::move(cb), button = getHandle()] { button->set_label(cb()); });
  // }
  //
  // void Button::setModifier(Text t) const
  // {
  //   getHandle()->set_label(t.text);
  // }
  //
  // void Button::setModifier(LabelCrop c) const
  // {
  //   if(const auto label = getLabel())
  //   {
  //     label->set_ellipsize(to<Pango::EllipsizeMode>(c.it));
  //   }
  // }
  //
  // Gtk::Label *Button::getLabel() const
  // {
  //   return dynamic_cast<Gtk::Label *>(getHandle()->get_child());
  // }
}