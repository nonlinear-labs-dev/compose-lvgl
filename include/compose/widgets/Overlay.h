#pragma once

#include "Widget.h"

namespace Compose
{
  // A modal layer on lv_layer_top(): a screen-filling backdrop swallowing all
  // input, with a centered content container the OVERLAY body composes into.
  // The backdrop's lifetime is tied to the creating widget via a hidden anchor
  // child, so tearing down or rebuilding the creator removes the overlay.
  class Overlay : public Widget
  {
   public:
    using Widget::setModifier;
    using Widget::WidgetType;

    // The content container lives on lv_layer_top() but keeps the creator's
    // style scope, so the overlay's YAML selectors nest under the creating
    // widget's path just like any child widget.
    template <typename... tArgs>
    explicit Overlay(Widget &creator, tArgs &&...args)
        : Widget(makeContent(creator))
    {
      setModifier(SizeVariant::FIT_CONTENT());
      setModifiers(this, creator, std::forward<tArgs>(args)...);
      lv_obj_center(getHandle());
    }

    explicit Overlay(WidgetType *w)
        : Widget(w)
    {
    }

    using DismissedCB = std::function<bool(Position)>;

    struct Dismissed
    {
      Overlay *m_parent;

      // Assign-only: the backdrop's click handler is wired once in makeContent
      // and dispatches to this stored callback, so a recomposed OVERLAY body
      // may set it again without re-registering the LVGL event.
      template <typename CB> void operator<<(CB &&cb) const
      {
        Widget backdrop(lv_obj_get_parent(m_parent->getHandle()));
        backdrop.ensureDataForKeyExistsOwning<DismissedCB>(c_dismissedKey) = std::forward<CB>(cb);
      }
    } dismissed { this };

   private:
    static constexpr auto c_dismissedKey = "OverlayDismissed";
    static WidgetType *makeContent(Widget &creator);
  };
}

#define OVERLAY(...) it.add(std::move(Compose::Overlay(it __VA_OPT__(, __VA_ARGS__)))) << [=](Compose::Overlay && it)
#define OVERLAY_DISMISSED it.dismissed << [=]
