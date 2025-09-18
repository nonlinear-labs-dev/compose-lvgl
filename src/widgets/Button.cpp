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
      Label(label).setModifier(t);
    }
  }

  void Button::setModifier(const Font &t) const
  {
    if(const auto label = getOrCreateLabel())
    {
      Label(label).setModifier(t);
    }
  }

  void Button::setModifier(ButtonType t) const
  {
    lv_obj_set_flag(getHandle(), LV_OBJ_FLAG_CHECKABLE, t.it == ButtonType::TOGGLE);
  }

  void Button::setModifier(PrimaryColor color) const
  {
    if(const auto label = getOrCreateLabel())
    {
      Label(label).setModifier(color);
    }
  }

  void Button::setModifier(const TextAlign &t) const
  {
    if(const auto label = getOrCreateLabel())
    {
      Label(label).setModifier(t);
    }
  }

  lv_obj_t *Button::getOrCreateLabel() const
  {
    const auto handle = getHandle();
    if(lv_obj_get_child_count(handle) == 0)
    {
      Label l(const_cast<Button &>(*this), Height::FULL());
    }

    auto childCount = lv_obj_get_child_count(handle);
    assert(childCount == 1);
    return lv_obj_get_child(handle, 0);
  }
}