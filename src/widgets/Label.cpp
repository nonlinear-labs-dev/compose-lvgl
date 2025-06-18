#include <compose/widgets/Label.h>
#include <reactive/Computations.h>

namespace Compose
{
  Label::Label(WidgetType *handle)
      : Widget(handle)
  {
  }

  void Label::setModifier(Text t) const
  {
    lv_label_set_text(getHandle(), t.text.c_str());
  }

  void Label::setModifier(PrimaryColor c) const
  {
    lv_obj_set_style_text_color(getHandle(),
                                {
                                    .blue = c.b,
                                    .green = c.g,
                                    .red = c.r,
                                },
                                LV_PART_MAIN);
    lv_obj_set_style_text_opa(getHandle(), static_cast<unsigned char>(c.a * 255.0), LV_PART_MAIN);
  }

  // void Label::setModifier(XAlign x) const
  // {
  //   switch(x.it)
  //   {
  //     case XAlignment::left:
  //       getHandle()->set_xalign(0);
  //       break;
  //     case XAlignment::center:
  //       getHandle()->set_xalign(0.5);
  //       break;
  //     case XAlignment::right:
  //       getHandle()->set_xalign(1);
  //       break;
  //   }
  // }
  //
  // void Label::setModifier(LabelCrop c) const
  // {
  //   getHandle()->set_ellipsize(to<Pango::EllipsizeMode>(c.it));
  // }
  //
  // void Label::setModifier(Justify j) const
  // {
  //   getHandle()->set_justify(to<Gtk::Justification>(j.it));
  // }
  //
  // void Label::setModifier(LetterSpacing j) const
  // {
  //   auto attr_list = getHandle()->get_attributes();
  //   if(!attr_list)
  //     attr_list = Pango::AttrList();
  //   auto size_attr = Pango::Attribute::create_attr_letter_spacing(j.it * PANGO_SCALE);
  //   attr_list.insert(size_attr);
  //   getHandle()->set_attributes(attr_list);
  // }
  //
  // void Label::setModifier(FontSize fontSize) const
  // {
  //   auto attr_list = getHandle()->get_attributes();
  //   if(!attr_list)
  //     attr_list = Pango::AttrList();
  //   auto size_attr = Pango::Attribute::create_attr_size(fontSize.it * PANGO_SCALE);
  //   attr_list.insert(size_attr);
  //   getHandle()->set_attributes(attr_list);
  // }
  //
  // void Label::setModifier(FontWeight w) const
  // {
  //   auto attr_list = getHandle()->get_attributes();
  //   if(!attr_list)
  //     attr_list = Pango::AttrList();
  //   auto attr = Pango::Attribute::create_attr_weight(static_cast<Pango::Weight>(w.it));
  //   attr_list.insert(attr);
  //   getHandle()->set_attributes(attr_list);
  // }
  //
  // void Label::setModifier(MonoSpaceNumbers m) const
  // {
  //   auto attr_list = getHandle()->get_attributes();
  //   if(!attr_list)
  //     attr_list = Pango::AttrList();
  //   if(m.it)
  //   {
  //     auto tabular_nums_attr = Pango::Attribute::create_attr_font_features("tnum");
  //     attr_list.insert(tabular_nums_attr);
  //   }
  //   else
  //   {
  //     auto tabular_nums_attr = Pango::Attribute::create_attr_font_features("");
  //     attr_list.change(tabular_nums_attr);
  //   }
  //   getHandle()->set_attributes(attr_list);
  // }

  void Label::operator<<(AutorunStringCB &&cb) const
  {
    // doAutorun([cb = std::move(cb), label = getHandle()] { label->set_text(cb()); });
  }
}
