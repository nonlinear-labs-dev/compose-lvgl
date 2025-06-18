#pragma once
#include "Align.h"
#include "Margin.h"
#include "Padding.h"
#include "Justify.h"
#include "AttachOptions.h"
#include "Orientation.h"
#include "Expand.h"
#include "Homogeneous.h"
#include "LabelCrop.h"
#include "Text.h"
#include "FontSize.h"
#include "FontWeight.h"
#include "Spacing.h"
#include "Size.h"
#include "BackgroundColor.h"
#include "PrimaryColor.h"
#include "SVGPath.h"
#include "PixelSize.h"
#include "Hidden.h"
#include "Position.h"
#include "MonoSpaceNumbers.h"

template <typename tOut, typename tIn> tOut to(tIn in)
{
  return static_cast<tOut>(in);
}

namespace Compose
{
  void setModifier(const auto &someWidget, auto someModifier)
  {
    someWidget.setModifier(someModifier);
  }

  struct ListEntryID
  {
    std::string id;
  };
}

#define HALIGN(...) it.doAutorun([=] { it.setModifier(HAlign(__VA_ARGS__)); });
#define VALIGN(...) it.doAutorun([=] { it.setModifier(VAlign(__VA_ARGS__)); });
#define MARGIN(...) it.doAutorun([=] { it.setModifier(Margin(__VA_ARGS__)); });
#define JUSTIFY(...) it.doAutorun([=] { it.setModifier(Justify(__VA_ARGS__)); });
#define ATTACH_OPTIONS(...) it.doAutorun([=] { it.setModifier(AttachOptions(__VA_ARGS__)); });
#define ORIENTATION(...) it.doAutorun([=] { it.setModifier(Orientation(__VA_ARGS__)); });
#define EXPAND(...) it.doAutorun([=] { it.setModifier(Expand(__VA_ARGS__)); });
#define HOMOGENEOUS(...) it.doAutorun([=] { it.setModifier(Homogeneous(__VA_ARGS__)); });
#define ROW_HOMOGENEOUS(...) it.doAutorun([=] { it.setModifier(RowHomogeneous(__VA_ARGS__)); });
#define COLUMN_HOMOGENEOUS(...) it.doAutorun([=] { it.setModifier(ColumnHomogeneous(__VA_ARGS__)); });
#define LABEL_CROP(...) it.doAutorun([=] { it.setModifier(LabelCrop(__VA_ARGS__)); });
#define STYLE_SHEET(...) it.doAutorun([=] { it.setModifier(StyleSheet(__VA_ARGS__)); });
#define LIST_ID(...) it.doAutorun([=] { it.setModifier(ListEntryID(__VA_ARGS__)); });
#define TEXT(...) it.doAutorun([=] { it.setModifier(Text(__VA_ARGS__)); });
#define FONT_SIZE(...) it.doAutorun([=] { it.setModifier(FontSize(__VA_ARGS__)); });
#define FONT_WEIGHT(...) it.doAutorun([=] { it.setModifier(FontWeight(__VA_ARGS__)); });
#define SPACING(...) it.doAutorun([=] { it.setModifier(Spacing(__VA_ARGS__)); });
#define ROW_SPACING(...) it.doAutorun([=] { it.setModifier(RowSpacing(__VA_ARGS__)); });
#define COLUMN_SPACING(...) it.doAutorun([=] { it.setModifier(ColumnSpacing(__VA_ARGS__)); });
#define FIXED_SIZE(...) it.doAutorun([=] { it.setModifier(FixedSize(__VA_ARGS__)); });
#define MIN_SIZE(...) it.doAutorun([=] { it.setModifier(MinSize(__VA_ARGS__)); });
#define BACKGROUND_COLOR(...) it.doAutorun([=] { it.setModifier(BackgroundColor(__VA_ARGS__)); });
#define PRIMARY_COLOR(...) it.doAutorun([=] { it.setModifier(PrimaryColor(__VA_ARGS__)); });
#define SVG_PATH(...) it.doAutorun([=] { it.setModifier(SVGPath(__VA_ARGS__)); });
#define READONLY(...) it.doAutorun([=] { it.setModifier(Readonly(__VA_ARGS__)); });
#define ROUNDED_CORNERS(...) it.doAutorun([=] { it.setModifier(RoundedCorner(__VA_ARGS__)); });
#define PADDING(...) it.doAutorun([=] { it.setModifier(Padding(__VA_ARGS__)); });
#define XALIGN(...) it.doAutorun([=] { it.setModifier(XAlign(__VA_ARGS__)); });
#define LETTER_SPACING(...) it.doAutorun([=] { it.setModifier(LetterSpacing(__VA_ARGS__)); });
#define HIDDEN(...) it.doAutorun([=] { it.setModifier(Hidden(__VA_ARGS__)); });
#define MONO_SPACE_NUMBERS(...) it.doAutorun([=] { it.setModifier(MonoSpaceNumbers(__VA_ARGS__)); });
#define POSITION(...) it.doAutorun([=] { it.setModifier(Position(__VA_ARGS__)); });