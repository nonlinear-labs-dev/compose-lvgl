#pragma once
#include "BaseWidget.h"
#include "compose/modifiers/Modifiers.h"
#include "handler/Clicks.h"
#include "compose/modifiers/OverflowBehaviour.h"
#include "compose/modifiers/Scrollable.h"

namespace Compose
{
  class Widget : public BaseWidget
  {
   public:
    using WidgetType = lv_obj_t;
    using super = BaseWidget;
    using super::ensureDataForKeyExistsNonOwning;
    using super::ensureDataForKeyExistsOwning;
    using super::getData;
    using super::getHandle;
    using AutorunCB = std::function<void(WidgetType &&)>;

    void applyDefaultStyle(auto *w)
    {
      static lv_style_t defaultStyle;
      lv_style_init(&defaultStyle);
      lv_style_set_radius(&defaultStyle, 0);
      lv_style_set_border_width(&defaultStyle, 0);
      lv_obj_add_style(w, &defaultStyle, LV_PART_MAIN);
    }

    explicit Widget(WidgetType *w)
        : BaseWidget(w)
    {
      applyDefaultStyle(w);
      setModifier(Scrollable(Scrollable::FIXED));
    }

    [[nodiscard]] int getWidth() const
    {
      return lv_obj_get_width(getHandle());
    }

    [[nodiscard]] int getHeight() const
    {
      return lv_obj_get_height(getHandle());
    }

    ~Widget() override = default;

    template <typename T> static T &&add(T &&w)
    {
      return std::forward<T>(w);
    }

    virtual void clear()
    {
      lv_obj_clean(getHandle());
    }

    void setModifier(OverflowBehaviour r) const
    {
      lv_obj_set_flag(getHandle(), LV_OBJ_FLAG_OVERFLOW_VISIBLE, r.it == OverflowBehaviour::VISIBLE);
    }

    void setModifier(Scrollable r) const
    {
      lv_obj_set_flag(getHandle(), LV_OBJ_FLAG_SCROLLABLE, r.it == Scrollable::SCROLLABLE);
    }

    void setModifier(BackgroundColor col) const
    {
      const auto opacity = static_cast<unsigned char>(col.a * 255.0);
      lv_obj_set_style_bg_color(getHandle(),
                                lv_color_t {
                                    .blue = col.b,
                                    .green = col.g,
                                    .red = col.r,
                                },
                                LV_PART_MAIN);
      lv_obj_set_style_bg_opa(getHandle(), opacity, LV_PART_MAIN);
    }

    void setModifier(FixedSize size) const
    {
      lv_obj_set_size(getHandle(), size.w, size.h);
    }

    std::shared_ptr<LeftClick<Widget>> leftClickHandler = std::make_shared<LeftClick<Widget>>(*this);
  };

  template <typename T>
  concept IsWidget = requires { typename T::WidgetType; };

  template <typename ComposeWidget, typename tCB>
    requires IsWidget<ComposeWidget>
  void operator<<(ComposeWidget &&lhs, tCB &&cb)
  {
    using tComposeWidgetDecayed = std::remove_reference_t<ComposeWidget>;

    lhs.doAutorun(
        [cb = std::forward<tCB>(cb), w = lhs.getHandle()]
        {
          tComposeWidgetDecayed wrapper(w);
          wrapper.clear();
          cb(tComposeWidgetDecayed(w));
        });
  }
}
