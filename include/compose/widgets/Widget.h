#pragma once
#include "BaseWidget.h"
#include "compose/modifiers/Modifiers.h"
#include "handler/Handlers.h"
#include "compose/modifiers/OverflowBehaviour.h"
#include "compose/modifiers/RoundedCorner.h"
#include "compose/modifiers/Scrollable.h"
#include "nltools/Assert.h"

#include <cassert>

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
      lv_style_set_flex_grow(&defaultStyle, 1);
      lv_obj_add_style(w, &defaultStyle, LV_PART_MAIN);
      lv_obj_set_size(getHandle(), 0, 0);
      lv_obj_add_flag(getHandle(), LV_OBJ_FLAG_CLICKABLE);
    }

    template <typename... tArgs>
    explicit Widget(Window &it, tArgs... args)
        : BaseWidget(lv_obj_create(nullptr))
    {
      lv_screen_load(BaseWidget::getHandle());
      applyDefaultStyle(BaseWidget::getHandle());
      setModifier(BackgroundColor::TRANSPARENT());
      (setModifier(args), ...);
    }

    template <typename... tArgs>
    explicit Widget(BaseWidget &w, tArgs... args)
        : Widget(lv_obj_create(w.getHandle()))
    {
      applyDefaultStyle(BaseWidget::getHandle());
      setModifier(BackgroundColor::TRANSPARENT());
      (setModifier(args), ...);
    }

    explicit Widget(WidgetType *w)
        : BaseWidget(w)
    {
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
      clearUserData();
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

    virtual void setModifier(BackgroundColor col) const
    {
      setColor(col);
    }

    void setSize(Size s) const
    {
      lv_obj_set_style_flex_grow(getHandle(), 0, LV_PART_MAIN);
      lv_obj_set_width(getHandle(), s.w);
      lv_obj_set_height(getHandle(), s.h);
    }

    void setModifier(FixedSize size) const
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
      if(const auto parent = lv_obj_get_parent(getHandle()))
      {

        try
        {
          nltools_detailedAssertAlways(lv_obj_get_style_layout(parent, LV_PART_MAIN) == LV_LAYOUT_NONE,
                                       "position only works with LAYOUT_TYPE NONE");
        }
        catch(std::exception &e)
        {
          throw;
        }
      }
      lv_obj_set_pos(getHandle(), pos.x, pos.y);
    }

    void setModifier(FlexAlign align) const
    {
      lv_obj_set_flex_align(getHandle(), align.main, align.cross, align.track_cross);
    }

    void setModifier(Align align) const
    {
      lv_obj_set_align(getHandle(), align.it);
    }

    void setModifier(Padding padding) const
    {
      lv_obj_set_style_pad_left(getHandle(), padding.left, LV_PART_MAIN);
      lv_obj_set_style_pad_top(getHandle(), padding.top, LV_PART_MAIN);
      lv_obj_set_style_pad_right(getHandle(), padding.right, LV_PART_MAIN);
      lv_obj_set_style_pad_bottom(getHandle(), padding.bottom, LV_PART_MAIN);
    }

    void setModifier(Margin margin) const
    {
      lv_obj_set_style_margin_left(getHandle(), margin.left, LV_PART_MAIN);
      lv_obj_set_style_margin_top(getHandle(), margin.top, LV_PART_MAIN);
      lv_obj_set_style_margin_right(getHandle(), margin.right, LV_PART_MAIN);
      lv_obj_set_style_margin_bottom(getHandle(), margin.bottom, LV_PART_MAIN);
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

    void setModifier(RoundedCorner corner) const
    {
      lv_obj_set_style_radius(getHandle(), corner.radius, LV_PART_MAIN);
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

    void setModifier(SizeVariant v) const
    {
      std::visit([this](auto &&it) { setModifier(it); }, v.it);
    }

    void setModifier(SizePercentage s) const
    {
      lv_obj_set_style_flex_grow(getHandle(), 0, LV_PART_MAIN);
      lv_obj_set_size(getHandle(), lv_pct(s.w), lv_pct(s.h));
    }

    void setModifier(Width w) const
    {
      if(const auto parent = lv_obj_get_parent(getHandle());
         lv_obj_get_style_flex_flow(parent, LV_PART_MAIN) == LV_FLEX_FLOW_ROW)
      {
        lv_obj_set_style_flex_grow(getHandle(), 0, LV_PART_MAIN);
      }
      lv_obj_set_width(getHandle(), w.it);
    }

    void setModifier(Height h) const
    {
      if(const auto parent = lv_obj_get_parent(getHandle());
         lv_obj_get_style_flex_flow(parent, LV_PART_MAIN) == LV_FLEX_FLOW_COLUMN)
      {
        lv_obj_set_style_flex_grow(getHandle(), 0, LV_PART_MAIN);
      }
      lv_obj_set_height(getHandle(), h.it);
    }

    void setModifier(FlexGrow g) const
    {
      lv_obj_set_style_flex_grow(getHandle(), g.it, LV_PART_MAIN);
    }

    struct Name
    {
      std::string name;
    };

    void setModifier(const Name &n) const
    {
      setID(n.name);
    }

    LeftClick leftClick { *this, c_leftClickKey };
    LongClick longClick { *this, c_longClickKey };
    StateChange stateChange { *this };
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

#define LEFT_CLICK() it.leftClick << [=]
#define LONG_CLICK() it.longClick << [=]
#define STATE_CHANGE it.stateChange << [=]
#define CLICK_TRACE()                                                                                                  \
  it.leftClick << [handle = it.getHandle()]                                                                            \
  {                                                                                                                    \
    nltools::Log::error(std::format("Our ID: {}", BaseWidget(handle).getID()));                                        \
    if(const auto parent = lv_obj_get_parent(handle))                                                                  \
    {                                                                                                                  \
      nltools::Log::error(std::format("Parent ID: {}", BaseWidget(parent).getID()));                                   \
    }                                                                                                                  \
    nltools::Log::error("width", lv_obj_get_width(handle), "height", lv_obj_get_height(handle));                       \
  }
