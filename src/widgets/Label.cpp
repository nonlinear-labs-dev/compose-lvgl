#include <compose/widgets/Label.h>
#include <reactive/Computations.h>
#include <cassert>
#include <utility>

namespace Compose
{
  std::unique_ptr<FontStorage> s_fontStorage = nullptr;

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

  void Label::setModifier(FontSize s) const
  {
    lv_obj_set_style_text_font(getHandle(), s_fontStorage->getFont(s).m_font, LV_PART_MAIN);
  }

  void Label::operator<<(AutorunStringCB &&cb) const
  {
    doAutorun([cb = std::move(cb), label = getHandle()] { lv_label_set_text(label, cb().c_str()); });
  }
}
