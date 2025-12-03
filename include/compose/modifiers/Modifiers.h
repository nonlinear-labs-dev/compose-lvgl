#pragma once
#include "Align.h"
#include "Margin.h"
#include "Padding.h"
#include "AttachOptions.h"
#include "FlexFlow.h"
#include "Expand.h"
#include "Text.h"
#include "Font.h"
#include "Spacing.h"
#include "Size.h"
#include "BackgroundColor.h"
#include "PrimaryColor.h"
#include "SVGPath.h"
#include "PixelSize.h"
#include "Hidden.h"
#include "Position.h"
#include <variant>

template <typename tOut, typename tIn> tOut to(tIn in)
{
  return static_cast<tOut>(in);
}

namespace Compose
{
  struct ListEntryID
  {
    std::string id;
  };

  struct SizePercentage
  {
    int w;
    int h;

    static constexpr SizePercentage FULL()
    {
      return { 100, 100 };
    }

    static constexpr SizePercentage PERCENTAGE(int s)
    {
      return { s, s };
    }

    bool operator==(const SizePercentage &) const = default;
  };

  struct Width
  {
    int it;

    static constexpr Width FULL()
    {
      return { LV_PCT(100) };
    }

    static constexpr Width PERCENT(auto i)
    {
      return { LV_PCT(i) };
    }

    static constexpr Width FIT_CONTENT()
    {
      return { LV_SIZE_CONTENT };
    }

    bool operator==(const Width &) const = default;
  };

  struct Height
  {
    int it;

    static constexpr Height FULL()
    {
      return { LV_PCT(100) };
    }

    static constexpr Height PERCENT(auto i)
    {
      return { LV_PCT(i) };
    }

    static constexpr Height FIT_CONTENT()
    {
      return { LV_SIZE_CONTENT };
    }

    bool operator==(const Height &) const = default;
  };

  struct FlexGrow
  {
    int it;

    static constexpr FlexGrow NONE()
    {
      return { 0 };
    }

    bool operator==(const FlexGrow &) const = default;
  };

  struct SizeVariant
  {
    explicit SizeVariant(auto it)
        : it(it)
    {
    }

    static SizeVariant FIT_CONTENT()
    {
      return SizeVariant { FixedSize { LV_SIZE_CONTENT, LV_SIZE_CONTENT } };
    }

    static SizeVariant FIXED(int w, int h)
    {
      return SizeVariant { FixedSize { w, h } };
    }

    static SizeVariant EXPAND_WIDTH(int fixedH)
    {
      return SizeVariant { FixedSize { LV_PCT(100), fixedH } };
    }

    static SizeVariant EXPAND_WIDTH()
    {
      return SizeVariant { FixedSize { LV_PCT(100), LV_SIZE_CONTENT } };
    }

    static SizeVariant EXPAND_HEIGHT(int fixedW)
    {
      return SizeVariant { FixedSize { fixedW, LV_PCT(100) } };
    }

    static SizeVariant EXPAND_HEIGHT()
    {
      return SizeVariant { FixedSize { LV_SIZE_CONTENT, LV_PCT(100) } };
    }

    static SizeVariant EXPAND_BOTH()
    {
      return SizeVariant { SizePercentage { 100, 100 } };
    }

    static SizeVariant PERCENTAGE(int w, int h)
    {
      return SizeVariant { SizePercentage { w, h } };
    }

    static SizeVariant WIDTH_FIXED(int w)
    {
      return SizeVariant { Width { w } };
    }

    static SizeVariant WIDTH_PERCENT(int w)
    {
      return SizeVariant { Width { LV_PCT(w) } };
    }

    static SizeVariant HEIGHT_FIXED(int h)
    {
      return SizeVariant { Height { h } };
    }

    static SizeVariant HEIGHT_PERCENT(int h)
    {
      return SizeVariant { Height { LV_PCT(h) } };
    }

    static SizeVariant FLEX_GROW(int g)
    {
      return SizeVariant { FlexGrow { g } };
    }

    std::variant<SizePercentage, FixedSize, Width, Height, FlexGrow> it;

    bool operator==(const SizeVariant &) const = default;
  };

  struct FlexAlign
  {
    lv_flex_align_t main;
    lv_flex_align_t cross;
    lv_flex_align_t track_cross;

    static constexpr FlexAlign CENTER()
    {
      return { LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER };
    }

    static constexpr FlexAlign START()
    {
      return { LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START };
    }

    static constexpr FlexAlign END()
    {
      return { LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END };
    }

    static constexpr FlexAlign CENTER_START()
    {
      return { LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START };
    }

    static constexpr FlexAlign CENTER_END()
    {
      return { LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END };
    }

    static constexpr FlexAlign START_END()
    {
      return { LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END };
    }

    static constexpr FlexAlign START_CENTER()
    {
      return { LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER };
    }

    static constexpr FlexAlign END_CENTER()
    {
      return { LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER };
    }

    static constexpr FlexAlign END_START()
    {
      return { LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_START };
    }

    static constexpr FlexAlign SPACE_EVENLY()
    {
      return { LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_SPACE_EVENLY };
    }

    bool operator==(const FlexAlign &) const = default;
  };

  struct TextAlign
  {
    lv_text_align_t it;

    static constexpr TextAlign CENTER()
    {
      return { LV_TEXT_ALIGN_CENTER };
    }

    static constexpr TextAlign LEFT()
    {
      return { LV_TEXT_ALIGN_LEFT };
    }

    static constexpr TextAlign RIGHT()
    {
      return { LV_TEXT_ALIGN_RIGHT };
    }

    bool operator==(const TextAlign &) const = default;
  };

  struct VerticalAlign
  {
    enum
    {
      Top,
      Center,
      Bottom
    } it;

    bool operator==(const VerticalAlign &) const = default;

    static constexpr VerticalAlign TOP()
    {
      return { Top };
    }

    static constexpr VerticalAlign CENTER()
    {
      return { Center };
    }

    static constexpr VerticalAlign BOTTOM()
    {
      return { Bottom };
    }
  };

  struct FlowDirection
  {
    enum
    {
      LeftToRight,
      RightToLeft
    } it;
  };

  struct FlexGap
  {
    int row;
    int column;
  };
}

#define ALIGN(...) it.doAutorun([=] { it.setModifier(Align(__VA_ARGS__)); });
#define TEXT_ALIGN(...) it.doAutorun([=] { it.setModifier(TextAlign { __VA_ARGS__ }); });
#define FLEX_ALIGN(...) it.doAutorun([=] { it.setModifier(FlexAlign { __VA_ARGS__ }); });
#define MARGIN(...) it.doAutorun([=] { it.setModifier(Margin(__VA_ARGS__)); });
#define ATTACH_OPTIONS(...) it.doAutorun([=] { it.setModifier(AttachOptions(__VA_ARGS__)); });
#define FLEX_FLOW(...) it.doAutorun([=] { it.setModifier(FlexFlow(__VA_ARGS__)); });
#define EXPAND(...) it.doAutorun([=] { it.setModifier(Expand(__VA_ARGS__)); });
#define STYLE_SHEET(...) it.doAutorun([=] { it.setModifier(StyleSheet(__VA_ARGS__)); });
#define LIST_ID(...) it.doAutorun([=] { it.setModifier(ListEntryID(__VA_ARGS__)); });
#define TEXT(...) it.doAutorun([=] { it.setModifier(Text(__VA_ARGS__)); });
#define FONT(...) it.doAutorun([=] { it.setModifier(Font { __VA_ARGS__ }); });
#define SPACING(...) it.doAutorun([=] { it.setModifier(Spacing(__VA_ARGS__)); });
#define ROW_SPACING(...) it.doAutorun([=] { it.setModifier(RowSpacing(__VA_ARGS__)); });
#define COLUMN_SPACING(...) it.doAutorun([=] { it.setModifier(ColumnSpacing(__VA_ARGS__)); });
#define FIXED_SIZE(...) it.doAutorun([=] { it.setModifier(FixedSize(__VA_ARGS__)); });
#define BACKGROUND_COLOR(...) it.doAutorun([=] { it.setModifier(BackgroundColor(__VA_ARGS__)); });
#define PRIMARY_COLOR(...) it.doAutorun([=] { it.setModifier(PrimaryColor(__VA_ARGS__)); });
#define SVG_PATH(...) it.doAutorun([=] { it.setModifier(SVGPath { __VA_ARGS__ }); });
#define SVG_CONTENT(...) it.doAutorun([=] { it.setModifier(SVGFileContent { __VA_ARGS__ }); });
#define READONLY(...) it.doAutorun([=] { it.setModifier(Readonly(__VA_ARGS__)); });
#define ROUNDED_CORNERS(...) it.doAutorun([=] { it.setModifier(RoundedCorner(__VA_ARGS__)); });
#define PADDING(...) it.doAutorun([=] { it.setModifier(Padding(__VA_ARGS__)); });
#define XALIGN(...) it.doAutorun([=] { it.setModifier(XAlign(__VA_ARGS__)); });
#define LETTER_SPACING(...) it.doAutorun([=] { it.setModifier(LetterSpacing(__VA_ARGS__)); });
#define HIDDEN(...) it.doAutorun([=] { it.setModifier(Hidden(__VA_ARGS__)); });
#define WIDTH(...) it.doAutorun([=] { it.setModifier(Width { __VA_ARGS__ }); });
#define HEIGHT(...) it.doAutorun([=] { it.setModifier(Height { __VA_ARGS__ }); });
#define POSITION(...) it.doAutorun([=] { it.setModifier(Position(__VA_ARGS__)); });
#define SCROLLABLE() it.doAutorun([=] { it.setModifier(Scrollable(Scrollable::SCROLLABLE)); });
#define CLICKABLE(b) it.doAutorun([=] { it.setModifier(Clickable(b)); });
#define NOT_SCROLLABLE() it.doAutorun([=] { it.setModifier(Scrollable(Scrollable::FIXED)); });
#define BUTTON_TYPE(...) it.doAutorun([=] { it.setModifier(ButtonType(__VA_ARGS__)); });
#define LAYOUT_TYPE(...) it.doAutorun([=] { it.setModifier(LayoutType(__VA_ARGS__)); });
#define SIZE(...) it.doAutorun([=] { it.setModifier(SizeVariant(__VA_ARGS__)); });
#define NAME(...) it.doAutorun([=] { it.setModifier(Widget::Name(__VA_ARGS__)); });
#define BORDER(...) it.doAutorun([=] { it.setModifier(Border(__VA_ARGS__)); });
#define FLEX_GROW(...) it.doAutorun([=] { it.setModifier(FlexGrow { __VA_ARGS__ }); });
#define VERTICAL_ALIGN(...) it.doAutorun([=] { it.setModifier(VerticalAlign(__VA_ARGS__)); });
#define FLOW_DIRECTION(...) it.doAutorun([=] { it.setModifier(FlowDirection(__VA_ARGS__)); });
#define FLEX_GAP(...) it.doAutorun([=] { it.setModifier(FlexGap { __VA_ARGS__ }); });
