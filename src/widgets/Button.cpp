#include <cassert>
#include <compose/widgets/Button.h>
#include <compose/widgets/Label.h>

namespace Compose
{
  void Button::setModifier(const Text &t) const
  {
    if(const auto label = getOrCreateLabel())
    {
      lv_label_set_text(label, t.text.c_str());
    }
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