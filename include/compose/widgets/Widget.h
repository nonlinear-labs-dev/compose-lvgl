#pragma once
#include "BaseWidget.h"
#include "compose/modifiers/Modifiers.h"
#include "handler/Clicks.h"
#include "compose/modifiers/OverflowBehaviour.h"
#include "compose/modifiers/RoundedCorner.h"
#include "compose/modifiers/Scrollable.h"

namespace Compose
{
  class Window;

  struct LayoutType
  {
    enum
    {
      NONE,
      FLEX,
    } it;

    static constexpr LayoutType none()
    {
      return { NONE };
    }

    static constexpr LayoutType flex()
    {
      return { FLEX };
    }
  };

  struct Border
  {
    int width;
    Color color;
  };

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
      lv_style_set_pad_all(&defaultStyle, 0);
      lv_style_set_pad_gap(&defaultStyle, 0);
      lv_style_set_margin_all(&defaultStyle, 0);
      lv_style_set_layout(&defaultStyle, LV_LAYOUT_FLEX);
      lv_obj_add_style(w, &defaultStyle, LV_PART_MAIN);
    }

    template <typename... tArgs>
    explicit Widget(Window &it, tArgs... args)
        : BaseWidget(lv_obj_create(nullptr))
    {
      lv_screen_load(BaseWidget::getHandle());
      (setModifier(args), ...);
    }

    template <typename... tArgs>
    explicit Widget(BaseWidget &w, tArgs... args)
        : Widget(lv_obj_create(w.getHandle()))
    {
      (setModifier(args), ...);
    }

    explicit Widget(WidgetType *w)
        : BaseWidget(w)
    {
      applyDefaultStyle(w);
      // setModifier(LayoutType::flex());
      // setModifier(Scrollable(Scrollable::FIXED));
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

    void setColor(Color col) const
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

    virtual void setModifier(PrimaryColor col) const
    {
      setColor(col);
    }

    void setModifier(BackgroundColor col) const
    {
      setColor(col);
    }

    void setSize(Size s) const
    {
      if(s.w > 0)
        lv_obj_set_width(getHandle(), s.w);
      if(s.h > 0)
        lv_obj_set_height(getHandle(), s.h);
    }

    void setModifier(FixedSize size) const
    {
      setSize(size);
    }

    [[deprecated]] void setModifier(MinSize size) const
    {
      setSize(size);
    }

    void setModifier(LayoutType r) const
    {
      lv_obj_set_layout(getHandle(), r.it == LayoutType::FLEX ? LV_LAYOUT_FLEX : LV_LAYOUT_NONE);
    }

    void setModifier(Orientation r) const
    {
      switch(r.it)
      {
        case OrientationEnum::HORIZONTAL:
          lv_obj_set_flex_flow(getHandle(), LV_FLEX_FLOW_ROW);
          break;
        case OrientationEnum::VERTICAL:
          lv_obj_set_flex_flow(getHandle(), LV_FLEX_FLOW_COLUMN);
          break;
      }
    }

    /*
    *
    *With default alignment it's the distance from the top left corner
E.g. LV_ALIGN_CENTER alignment it's the offset from the center of the parent
The position is interpreted on the content area of the parent
The values can be set in pixel or in percentage of parent size with lv_pct(v)
     */
    void setModifier(Position pos) const
    {
      lv_obj_set_pos(getHandle(), pos.x, pos.y);
    }

    void setModifier(Align align) const
    {
      lv_obj_set_align(getHandle(), align.it);
    }

    void setModifier(Padding padding) const
    {
      auto [left, top, right, bottom] = padding;
      lv_obj_set_style_pad_left(getHandle(), left, LV_PART_MAIN);
      lv_obj_set_style_pad_top(getHandle(), top, LV_PART_MAIN);
      lv_obj_set_style_pad_right(getHandle(), right, LV_PART_MAIN);
      lv_obj_set_style_pad_bottom(getHandle(), bottom, LV_PART_MAIN);
    }

    void setModifier(Margin margin) const
    {
      auto [left, top, right, bottom] = margin;
      lv_obj_set_style_margin_left(getHandle(), left, LV_PART_MAIN);
      lv_obj_set_style_margin_top(getHandle(), top, LV_PART_MAIN);
      lv_obj_set_style_margin_right(getHandle(), right, LV_PART_MAIN);
      lv_obj_set_style_margin_bottom(getHandle(), bottom, LV_PART_MAIN);
    }

    void setModifier(Border border) const
    {
      lv_obj_set_style_border_width(getHandle(), border.width, LV_PART_MAIN);
      lv_obj_set_style_border_color(getHandle(),
                                    lv_color_t {
                                        .blue = border.color.b,
                                        .green = border.color.g,
                                        .red = border.color.r,
                                    },
                                    LV_PART_MAIN);
      lv_obj_set_style_border_opa(getHandle(), border.color.a * 255, LV_PART_MAIN);
    }

    //sadly only all borders share the same radius
    void setModifier(RoundedCorner corner) const
    {
      const auto max
          = std::max(std::max(std::max(corner.topLeft, corner.topRight), corner.bottomLeft), corner.bottomRight);
      lv_obj_set_style_radius(getHandle(), max, LV_PART_MAIN);
    }

    void setModifier(Hidden h) const
    {
      lv_obj_set_flag(getHandle(), LV_OBJ_FLAG_HIDDEN, h.it);
    }

    void setModifier(Expand e) const
    {
      if(e.horizontal)
      {
        lv_obj_set_width(getHandle(), LV_PCT(100));
      }
      if(e.vertical)
      {
        lv_obj_set_height(getHandle(), LV_PCT(100));
      }
    }

    void setModifier(Homogeneous h) const
    {
      nltools::Log::error(__PRETTY_FUNCTION__, "not implemented");
    }

    void setModifier(FitContent c) const
    {
      if(c.it)
      {
        lv_obj_set_size(getHandle(), LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      }
      else
      {
        lv_obj_set_size(getHandle(), getWidth(), getHeight());
      }
    }

    void setModifier(SizeVariant v) const
    {
      std::visit([this](auto &&it) { setModifier(it); }, v.it);
    }

    void setModifier(SizePercentage s) const
    {
      lv_obj_set_size(getHandle(), lv_pct(s.w), lv_pct(s.h));
    }

    std::shared_ptr<LeftClick<Widget>> leftClickHandler = std::make_shared<LeftClick<Widget>>(*this);
    std::shared_ptr<StateChange<Widget>> stateChangeHandler = std::make_shared<StateChange<Widget>>(*this);
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
