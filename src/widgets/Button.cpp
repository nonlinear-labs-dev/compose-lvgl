#include <cassert>
#include <compose/widgets/Button.h>
#include <compose/widgets/Label.h>
#include <src/widgets/button/lv_button.h>

namespace Compose
{
  void Button::setModifier(const Text &t) const
  {
    if(const auto label = getOrCreateLabel())
    {
      lv_label_set_text(label, t.text.c_str());
    }
  }

  void Button::setModifier(ButtonType t) const
  {
    lv_obj_set_flag(getHandle(), LV_OBJ_FLAG_CHECKABLE, t.it == ButtonType::TOGGLE);
  }

  lv_obj_t *Button::getOrCreateLabel() const
  {
    const auto handle = getHandle();
    if(lv_obj_get_child_count(handle) == 0)
    {
      lv_obj_center(lv_label_create(handle));
    }

    assert(lv_obj_get_child_count(handle) == 1);
    return lv_obj_get_child(handle, 0);
  }
}