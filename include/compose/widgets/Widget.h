#pragma once
#include "BaseWidget.h"
#include "compose/modifiers/Clickable.h"
#include "compose/modifiers/FlexFlow.h"
#include "compose/modifiers/Modifiers.h"
#include "handler/DragDrop.h"
#include "handler/Handlers.h"
#include "compose/modifiers/OverflowBehaviour.h"
#include "compose/modifiers/RoundedCorner.h"
#include "compose/modifiers/Scrollable.h"
#include "compose/modifiers/StyleSheets.h"

#include <stdexcept>
#include <type_traits>

namespace Compose
{
  namespace Detail
  {
    template <typename Tuple> struct StyleClassCollector
    {
      Tuple classes;

      StyleClassCollector(const Tuple &c)
          : classes(c)
      {
      }

      auto add(const auto &c)
      {
        return *this;
      }

      auto add(const StyleClass &c)
      {
        auto r = std::tuple_cat(classes, std::make_tuple(c.name));
        return StyleClassCollector<decltype(r)> { r };
      }

      template <typename tArgs> auto operator+(const tArgs &arg)
      {
        return add(arg);
      }
    };

    template <typename... tArgs> void applyStyleClasses(Style &style, tArgs &&... args)
    {
      StyleClassCollector s(std::make_tuple());
      auto classes = (s + ... + args);
      std::apply([&](auto &... args) { style.add(args...); }, classes.classes);
    }
  }

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

    bool operator==(const LayoutType &) const = default;
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

    void setDefaultWidthAndHeightAccordingToParent() const
    {
      if(const auto parent = lv_obj_get_parent(getHandle()))
      {
        if(lv_obj_get_style_flex_flow(parent, LV_PART_MAIN) == LV_FLEX_FLOW_COLUMN)
        {
          auto parentWidth = lv_obj_get_style_width(parent, LV_PART_MAIN);
          auto defaultWidth = Width::FULL();
          if(parentWidth == LV_SIZE_CONTENT)
          {
            defaultWidth = Width::FIT_CONTENT();
          }
          setModifier(defaultWidth);
        }
        else if(lv_obj_get_style_flex_flow(parent, LV_PART_MAIN) == LV_FLEX_FLOW_ROW)
        {
          auto parentHeight = lv_obj_get_style_height(parent, LV_PART_MAIN);
          auto defaultHeight = Height::FULL();
          if(parentHeight == LV_SIZE_CONTENT)
          {
            defaultHeight = Height::FIT_CONTENT();
          }
          setModifier(defaultHeight);
        }
      }
    }

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
      lv_obj_add_flag(getHandle(), LV_OBJ_FLAG_EVENT_BUBBLE);
    }

    template <typename... tArgs>
    explicit Widget(Window &it, tArgs &&... args)
        : BaseWidget(lv_obj_create(nullptr))
    {
      lv_screen_load(BaseWidget::getHandle());
      applyDefaultStyle(BaseWidget::getHandle());
      Widget::setModifier(BackgroundColor::TRANSPARENT());
      (setModifier(std::forward<tArgs>(args)), ...);
    }

    template <typename... tArgs>
    explicit Widget(BaseWidget &w, tArgs &&... args)
        : Widget(lv_obj_create(w.getHandle()))
    {
      applyDefaultStyle(BaseWidget::getHandle());
      setDefaultWidthAndHeightAccordingToParent();
      Widget::setModifier(BackgroundColor::TRANSPARENT());

      auto parentStyle = Widget(w.getHandle()).getStyle().inherit();
      Detail::applyStyleClasses(parentStyle, args...);
      setModifier(parentStyle);
      (setModifier(std::forward<tArgs>(args)), ...);
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

    [[nodiscard]] virtual bool shouldClearBeforeAutorunCompose() const
    {
      return true;
    }

    void setModifier(StyleClass c)
    {
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

    void setModifier(FlexFlow r) const
    {
      switch(r.it)
      {
        case FlexFlowEnum::HORIZONTAL:
          lv_obj_set_flex_flow(getHandle(), LV_FLEX_FLOW_ROW);
          break;
        case FlexFlowEnum::HORIZONTAL_REVERSE:
          lv_obj_set_flex_flow(getHandle(), LV_FLEX_FLOW_ROW_REVERSE);
          break;
        case FlexFlowEnum::HORIZONTAL_WRAP:
          lv_obj_set_flex_flow(getHandle(), LV_FLEX_FLOW_ROW_WRAP);
          break;
        case FlexFlowEnum::HORIZONTAL_WRAP_REVERSE:
          lv_obj_set_flex_flow(getHandle(), LV_FLEX_FLOW_ROW_WRAP_REVERSE);
          break;
        case FlexFlowEnum::VERTICAL:
          lv_obj_set_flex_flow(getHandle(), LV_FLEX_FLOW_COLUMN);
          break;
        case FlexFlowEnum::VERTICAL_REVERSE:
          lv_obj_set_flex_flow(getHandle(), LV_FLEX_FLOW_COLUMN_REVERSE);
          break;
        case FlexFlowEnum::VERTICAL_WRAP:
          lv_obj_set_flex_flow(getHandle(), LV_FLEX_FLOW_COLUMN_WRAP);
          break;
        case FlexFlowEnum::VERTICAL_WRAP_REVERSE:
          lv_obj_set_flex_flow(getHandle(), LV_FLEX_FLOW_COLUMN_WRAP_REVERSE);
          break;
      }
    }

    void setModifier(Position pos) const
    {
      if(const auto parent = lv_obj_get_parent(getHandle()))
      {
        try
        {
          assert(lv_obj_get_style_layout(parent, LV_PART_MAIN) == LV_LAYOUT_NONE);  // position only works with LAYOUT_TYPE NONE
        }
        catch([[maybe_unused]] std::exception &e)
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

    void setModifier(MarginLeft m) const
    {
      lv_obj_set_style_margin_left(getHandle(), m.margin, LV_PART_MAIN);
    }

    void setModifier(MarginTop m) const
    {
      lv_obj_set_style_margin_top(getHandle(), m.margin, LV_PART_MAIN);
    }

    void setModifier(MarginRight m) const
    {
      lv_obj_set_style_margin_right(getHandle(), m.margin, LV_PART_MAIN);
    }

    void setModifier(MarginBottom m) const
    {
      lv_obj_set_style_margin_bottom(getHandle(), m.margin, LV_PART_MAIN);
    }

    void setModifier(Border border) const
    {
      setModifier(BorderWidth { border.width });
      setModifier(BorderColor { border.color });
    }

    void setModifier(BorderWidth border) const
    {
      lv_obj_set_style_border_width(getHandle(), border.width, LV_PART_MAIN);
    }

    void setModifier(BorderColor border) const
    {
      lv_obj_set_style_border_color(getHandle(),
                                    lv_color_t {
                                        .blue = border.color.b,
                                        .green = border.color.g,
                                        .red = border.color.r,
                                    },
                                    LV_PART_MAIN);
      lv_obj_set_style_border_opa(getHandle(), static_cast<unsigned short>(border.color.a * 255), LV_PART_MAIN);
    }

    void setModifier(const BorderSides &sides) const
    {
      int value = LV_BORDER_SIDE_NONE;

      for(auto s : sides.sides)
      {
        switch(s)
        {
          case BorderSides::TOP:
            value |= (int) LV_BORDER_SIDE_TOP;
            break;
          case BorderSides::BOTTOM:
            value |= (int) LV_BORDER_SIDE_BOTTOM;
            break;
          case BorderSides::LEFT:
            value |= (int) LV_BORDER_SIDE_LEFT;
            break;
          case BorderSides::RIGHT:
            value |= (int) LV_BORDER_SIDE_RIGHT;
            break;
        }
      }
      lv_obj_set_style_border_side(getHandle(), static_cast<lv_border_side_t>(value), LV_PART_MAIN);
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
      if(auto parent = lv_obj_get_parent(getHandle()))
      {
        auto parentWidth = lv_obj_get_style_width(parent, LV_PART_MAIN);
        if(parentWidth == LV_SIZE_CONTENT)
        {
          LV_ASSERT_MSG(false, "Percent/FULL child width under FIT_CONTENT parent width creates a layout loop");
          throw std::logic_error("Percent/FULL child width under FIT_CONTENT parent width creates a layout loop");
        }

        auto parentHeight = lv_obj_get_style_height(parent, LV_PART_MAIN);
        if(parentHeight == LV_SIZE_CONTENT)
        {
          LV_ASSERT_MSG(false, "Percent/FULL child height under FIT_CONTENT parent height creates a layout loop");
          throw std::logic_error("Percent/FULL child height under FIT_CONTENT parent height creates a layout loop");
        }
      }

      lv_obj_set_style_flex_grow(getHandle(), 0, LV_PART_MAIN);
      lv_obj_set_size(getHandle(), lv_pct(s.w), lv_pct(s.h));
    }

    virtual void setModifier(Font s) const
    {
    }

    virtual void setModifier(TextAlign a) const
    {
    }

    virtual void setModifier(VerticalAlign v) const
    {
    }

    void setModifier(const Style &style) const
    {
      auto &s = ensureDataForKeyExistsOwning<Style>(c_styleKey);
      s = style;
      auto doNothing = [] {};
      std::apply([&](const auto &... a) { ((a.has_value() ? setModifier(a.value()) : doNothing()), ...); }, style.properties);
    }

    const Style &getStyle() const
    {
      return ensureDataForKeyExistsOwning<Style>(c_styleKey);
    }

    template <lv_flex_flow_t... values> static bool anyOf(lv_flex_flow_t v)
    {
      return ((v == values) || ...);
    }

    static bool isRow(lv_flex_flow_t v)
    {
      return anyOf<LV_FLEX_FLOW_ROW, LV_FLEX_FLOW_ROW_REVERSE, LV_FLEX_FLOW_ROW_WRAP, LV_FLEX_FLOW_ROW_WRAP_REVERSE>(v);
    }

    static bool isColumn(lv_flex_flow_t v)
    {
      return anyOf<LV_FLEX_FLOW_COLUMN, LV_FLEX_FLOW_COLUMN_REVERSE, LV_FLEX_FLOW_COLUMN_WRAP, LV_FLEX_FLOW_COLUMN_WRAP_REVERSE>(v);
    }

    virtual void setModifier(Width w) const
    {
      if(const auto parent = lv_obj_get_parent(getHandle()))
      {
        if(LV_COORD_IS_PCT(w.it))
        {
          auto parentWidth = lv_obj_get_style_width(parent, LV_PART_MAIN);
          if(parentWidth == LV_SIZE_CONTENT)
          {
            LV_ASSERT_MSG(false, "Percent/FULL child width under FIT_CONTENT parent width creates a layout loop");
            throw std::logic_error("Percent/FULL child width under FIT_CONTENT parent width creates a layout loop");
          }
        }

        if(isRow(lv_obj_get_style_flex_flow(parent, LV_PART_MAIN)))
        {
          lv_obj_set_style_flex_grow(getHandle(), 0, LV_PART_MAIN);
        }
      }
      lv_obj_set_width(getHandle(), w.it);
    }

    virtual void setModifier(Height h) const
    {
      if(const auto parent = lv_obj_get_parent(getHandle()))
      {
        if(LV_COORD_IS_PCT(h.it))
        {
          auto parentHeight = lv_obj_get_style_height(parent, LV_PART_MAIN);
          if(parentHeight == LV_SIZE_CONTENT)
          {
            LV_ASSERT_MSG(false, "Percent/FULL child height under FIT_CONTENT parent height creates a layout loop");
            throw std::logic_error("Percent/FULL child height under FIT_CONTENT parent height creates a layout loop");
          }
        }

        if(isColumn(lv_obj_get_style_flex_flow(parent, LV_PART_MAIN)))
        {
          lv_obj_set_style_flex_grow(getHandle(), 0, LV_PART_MAIN);
        }
      }
      lv_obj_set_height(getHandle(), h.it);
    }

    void setModifier(FlexGrow g) const
    {
      lv_obj_set_style_flex_grow(getHandle(), g.it, LV_PART_MAIN);
    }

    void setModifier(FlowDirection d) const
    {
      if(d.it == FlowDirection::LeftToRight)
      {
        lv_obj_set_style_base_dir(getHandle(), LV_BASE_DIR_LTR, LV_PART_MAIN);
      }
      else
      {
        lv_obj_set_style_base_dir(getHandle(), LV_BASE_DIR_RTL, LV_PART_MAIN);
      }
    }

    void setModifier(FlexGap gap) const
    {
      lv_obj_set_style_pad_row(getHandle(), gap.row, LV_PART_MAIN);
      lv_obj_set_style_pad_column(getHandle(), gap.column, LV_PART_MAIN);
    }

    struct Name
    {
      std::string name;
    };

    void setModifier(const Name &n) const
    {
      setID(n.name);
    }

    void setModifier(const Clickable &c) const
    {
      lv_obj_set_flag(getHandle(), LV_OBJ_FLAG_CLICKABLE, static_cast<bool>(c));
    }

    LeftClick leftClick { *this, c_leftClickKey };
    LongClick longClick { *this, c_longClickKey };
    Touch touch { *this };
    StateChange stateChange { *this };
    Drag drag { *this };
    DragDrop dragDrop { *this };
    TimerTick timerTick { *this };

    [[nodiscard]] bool isCurrentDropTarget() const
    {
      return DragDropContext::get().isCurrentTarget(getHandle());
    }
  };

  template <typename T> concept IsWidget = requires
  {
    typename T::WidgetType;
  };

  template <typename ComposeWidget, typename tCB> requires IsWidget<ComposeWidget> void operator<<(ComposeWidget &&lhs, tCB &&cb)
  {
    using tComposeWidgetDecayed = std::remove_reference_t<ComposeWidget>;

    lhs.doAutorun([cb = std::forward<tCB>(cb), w = lhs.getHandle()] {
      tComposeWidgetDecayed wrapper(w);
      if(wrapper.shouldClearBeforeAutorunCompose())
        wrapper.clear();
      cb(tComposeWidgetDecayed(w));
    });
  }
}

#define SCROLL_INTO_VIEW_WHEN(condition)                                                                                                                                           \
  it.doAutorun([=, handle = it.getHandle()] {                                                                                                                                      \
    if(condition)                                                                                                                                                                  \
    {                                                                                                                                                                              \
      lv_obj_scroll_to_view(handle, false);                                                                                                                                        \
    }                                                                                                                                                                              \
  });

#define LEFT_CLICK it.leftClick << [=]
#define TOUCH() it.touch << [=](Compose::Touch * it)
#define TOUCH_BEGIN it->begin << [=]
#define TOUCH_UPDATE it->update << [=]
#define TOUCH_END it->end << [=]
#define SWALLOW_LEFT_CLICK()                                                                                                                                                       \
  LEFT_CLICK(auto)                                                                                                                                                                 \
  {                                                                                                                                                                                \
    return true;                                                                                                                                                                   \
  };
#define LONG_CLICK it.longClick << [=]
#define STATE_CHANGE it.stateChange << [=]
#define CLICK_TRACE()                                                                                                                                                              \
  it.leftClick << [handle = it.getHandle()](Position p) -> bool {                                                                                                                  \
    nltools::Log::error(std::format("Clicked {} at {}/{}", BaseWidget(handle).getID(), p.x, p.y));                                                                                 \
    return false;                                                                                                                                                                  \
  }

#define WITH_STATE(type, name, factory)                                                                                                                                            \
  auto &_state_ref_##name                                                                                                                                                          \
      = it.ensureDataForKeyExistsOwning<Reactive::Var<type>>("WITH_STATE_" #name "_" #type, [inner_factory = factory] { return new Reactive::Var<type>(inner_factory()); });       \
  if(auto *name = &_state_ref_##name)

#define TIMER_TICK it.timerTick << [=]
