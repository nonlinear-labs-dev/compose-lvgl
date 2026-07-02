#pragma once
#include "Widget.h"
#include "handler/Handlers.h"
#include "src/widgets/button/lv_button.h"

#include <functional>
#include <string>

namespace Compose
{
  struct ButtonType
  {
    enum
    {
      NORMAL,
      TOGGLE
    } it;

    bool operator==(const ButtonType&) const = default;
  };

  class Button : public Widget
  {
   public:
    using Widget::setModifier;
    using Widget::Widget;

    template <typename... tArgs>
    explicit Button(Widget& parent, tArgs... args)
        : Widget(lv_button_create(parent.getHandle()))
    {
      applyDefaultStyle(BaseWidget::getHandle());
      setModifiers(this, parent, std::forward<tArgs>(args)...);
    }

    void setModifier(ButtonType t) const;
    void setModifier(const Text& t) const;
    void setModifier(Font t) const override;
    void setModifier(PrimaryColor color) const override;
    void setModifier(TextAlign t) const override;

   private:
    [[nodiscard]] lv_obj_t* getOrCreateLabel() const;
  };
}

#define BUTTON(...) it.add(Compose::Button(it __VA_OPT__(, __VA_ARGS__))) << [=](Compose::Button&& it)
