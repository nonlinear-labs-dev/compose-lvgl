#pragma once
#include "compose/widgets/Widget.h"

namespace Compose
{
  class Fixed final : public Widget<Gtk::Fixed>
  {
   public:
    using Widget::getHandle;
    using Widget::setModifier;

    template <typename... tArgs>
    explicit Fixed(tArgs... args)
        : Widget(Gtk::make_managed<Gtk::Fixed>())
    {
      (setModifier(args), ...);
    }

    explicit Fixed(WidgetType *handle);

    template <typename T> T &&add(T &&toAdd)
    {
      getHandle()->put(*toAdd.getHandle(), 0, 0);
      return std::forward<T>(toAdd);
    }

    void clear() override;
    void setModifier(FixedSize r) const;
  };
}

#define FIXED(...) it.add(Compose::Fixed(__VA_ARGS__)) << [=](Compose::Fixed &&it)
