#pragma once

#include <nlohmann/json.hpp>
#include "Border.h"
#include "Font.h"
#include "Modifiers.h"
#include "Padding.h"
#include "RoundedCorner.h"
#include "Scrollable.h"
#include <vector>
#include <filesystem>

namespace Compose
{
  struct StyleSheet;
  struct RoundedCorner;
  struct Scrollable;
  struct Expand;
  struct FlexFlow;
  struct FlexGap;
  struct FlexAlign;
  struct VerticalAlign;
  struct TextAlign;
  struct Font;
  struct BorderSides;
  struct BorderColor;
  struct BorderWidth;
  struct Border;
  struct Padding;
  struct MarginBottom;
  struct MarginTop;
  struct MarginRight;
  struct MarginLeft;
  struct Margin;
  struct Height;
  struct Width;
  class Color;

  namespace Detail
  {
    inline Font::FontWeight parseFontWeight(const std::string& weight)
    {
      if(weight == "Black")
        return Font::FontWeight::Black;
      if(weight == "ExtraBold")
        return Font::FontWeight::ExtraBold;
      if(weight == "Bold")
        return Font::FontWeight::Bold;
      if(weight == "SemiBold")
        return Font::FontWeight::SemiBold;
      if(weight == "Medium")
        return Font::FontWeight::Medium;
      if(weight == "Light")
        return Font::FontWeight::Light;

      return Font::FontWeight::Regular;
    }

    inline TextAlign parseTextAlign(const std::string& value)
    {
      if(value == "CENTER")
        return TextAlign::CENTER();
      if(value == "LEFT")
        return TextAlign::LEFT();
      if(value == "RIGHT")
        return TextAlign::RIGHT();

      throw std::invalid_argument("invalid TextAlign: " + value);
    }

    inline VerticalAlign parseVerticalAlign(const std::string& value)
    {
      if(value == "TOP")
        return VerticalAlign::TOP();
      if(value == "CENTER")
        return VerticalAlign::CENTER();
      if(value == "BOTTOM")
        return VerticalAlign::BOTTOM();

      throw std::invalid_argument("invalid VerticalAlign: " + value);
    }

    inline FlexAlign parseFlexAlign(const std::string& value)
    {
      if(value == "CENTER")
        return FlexAlign::CENTER();
      if(value == "START")
        return FlexAlign::START();
      if(value == "END")
        return FlexAlign::END();
      if(value == "CENTER_START")
        return FlexAlign::CENTER_START();
      if(value == "CENTER_END")
        return FlexAlign::CENTER_END();
      if(value == "START_END")
        return FlexAlign::START_END();
      if(value == "START_CENTER")
        return FlexAlign::START_CENTER();
      if(value == "END_CENTER")
        return FlexAlign::END_CENTER();
      if(value == "END_START")
        return FlexAlign::END_START();
      if(value == "SPACE_EVENLY")
        return FlexAlign::SPACE_EVENLY();
      if(value == "SPACE_BETWEEN")
        return FlexAlign::SPACE_BETWEEEN();

      throw std::invalid_argument("invalid FlexAlign: " + value);
    }

    inline FlexFlow parseFlexFlow(const std::string& value)
    {
      if(value == "HORIZONTAL")
        return FlexFlow::HORIZONTAL();
      if(value == "HORIZONTAL_REVERSE")
        return FlexFlow::HORIZONTAL_REVERSE();
      if(value == "HORIZONTAL_WRAP")
        return FlexFlow::HORIZONTAL_WRAP();
      if(value == "HORIZONTAL_WRAP_REVERSE")
        return FlexFlow::HORIZONTAL_WRAP_REVERSE();
      if(value == "VERTICAL")
        return FlexFlow::VERTICAL();
      if(value == "VERTICAL_REVERSE")
        return FlexFlow::VERTICAL_REVERSE();
      if(value == "VERTICAL_WRAP")
        return FlexFlow::VERTICAL_WRAP();
      if(value == "VERTICAL_WRAP_REVERSE")
        return FlexFlow::VERTICAL_WRAP_REVERSE();

      throw std::invalid_argument("invalid FlexFlow: " + value);
    }

    inline Expand parseExpand(const std::string& value)
    {
      if(value == "VERTICAL")
        return Expand::VERTICAL();
      if(value == "HORIZONTAL")
        return Expand::HORIZONTAL();
      if(value == "NONE")
        return Expand::NONE();
      if(value == "BOTH")
        return Expand::BOTH();

      throw std::invalid_argument("invalid Expand: " + value);
    }
  }

  template <typename tJson> void to_json(tJson& j, const Color& in)
  {
    j = "#" + in.toHEXString();
  }

  template <typename tJson> void from_json(const tJson& j, Color& out)
  {
    out = Color::fromHEXString(j.template get<std::string>());
  }

  template <typename tJson> void to_json(tJson& j, const Width& in)
  {
    if(in.it == Width::FIT_CONTENT().it)
    {
      j = "FIT_CONTENT";
      return;
    }

    if(in.it == Width::FULL().it)
    {
      j = "FULL";
      return;
    }

    j = in.it;
  }

  template <typename tJson> void from_json(const tJson& j, Width& out)
  {
    if(j.is_string())
    {
      const auto v = j.template get<std::string>();
      if(v == "FIT_CONTENT")
      {
        out = Width::FIT_CONTENT();
        return;
      }

      if(v == "FULL")
      {
        out = Width::FULL();
        return;
      }

      throw std::invalid_argument("invalid Width string: " + v);
    }

    out = Width { j.template get<int>() };
  }

  template <typename tJson> void to_json(tJson& j, const Height& in)
  {
    if(in.it == Height::FIT_CONTENT().it)
    {
      j = "FIT_CONTENT";
      return;
    }

    if(in.it == Height::FULL().it)
    {
      j = "FULL";
      return;
    }

    j = in.it;
  }

  template <typename tJson> void from_json(const tJson& j, Height& out)
  {
    if(j.is_string())
    {
      const auto v = j.template get<std::string>();
      if(v == "FIT_CONTENT")
      {
        out = Height::FIT_CONTENT();
        return;
      }

      if(v == "FULL")
      {
        out = Height::FULL();
        return;
      }

      throw std::invalid_argument("invalid Height string: " + v);
    }

    out = Height { j.template get<int>() };
  }

  template <typename tJson> void to_json(tJson& j, const Margin& in)
  {
    j = tJson::array({ in.top, in.bottom, in.left, in.right });
  }

  template <typename tJson> void to_json(tJson& j, const MarginLeft& in)
  {
    j = in.margin;
  }

  template <typename tJson> void to_json(tJson& j, const MarginRight& in)
  {
    j = in.margin;
  }

  template <typename tJson> void to_json(tJson& j, const MarginTop& in)
  {
    j = in.margin;
  }

  template <typename tJson> void to_json(tJson& j, const MarginBottom& in)
  {
    j = in.margin;
  }

  template <typename tJson> void from_json(const tJson& j, Margin& out)
  {
    if(j.is_number_integer())
    {
      out = Margin { j.template get<int>() };
      return;
    }

    if(j.is_array() && j.size() == 4)
    {
      out = Margin { j.at(0).template get<int>(), j.at(1).template get<int>(), j.at(2).template get<int>(), j.at(3).template get<int>() };
      return;
    }

    throw std::invalid_argument("Margin must be int or [top, bottom, left, right]");
  }

  template <typename tJson> void to_json(tJson& j, const Padding& in)
  {
    j = tJson::array({ in.top, in.bottom, in.left, in.right });
  }

  template <typename tJson> void from_json(const tJson& j, Padding& out)
  {
    if(j.is_number_integer())
    {
      out = Padding { j.template get<int>() };
      return;
    }

    if(j.is_array() && j.size() == 4)
    {
      out = Padding { j.at(0).template get<int>(), j.at(1).template get<int>(), j.at(2).template get<int>(), j.at(3).template get<int>() };
      return;
    }

    throw std::invalid_argument("Padding must be int or [top, bottom, left, right]");
  }

  template <typename tJson> void to_json(tJson& j, const Border& in)
  {
    j = tJson { { "width", in.width }, { "color", in.color } };
  }

  template <typename tJson> void from_json(const tJson& j, Border& out)
  {
    out = Border { .width = j.at("width").template get<int>(), .color = j.at("color").template get<Color>() };
  }

  template <typename tJson> void to_json(tJson& j, const BorderWidth& in)
  {
    j = in.width;
  }

  template <typename tJson> void to_json(tJson& j, const BorderColor& in)
  {
    j = in.color;
  }

  template <typename tJson> void to_json(tJson& j, const BorderSides& in)
  {
    j = tJson::array();
    for(const auto side : in.sides)
    {
      if(side == BorderSides::TOP)
        j.push_back("TOP");
      else if(side == BorderSides::BOTTOM)
        j.push_back("BOTTOM");
      else if(side == BorderSides::LEFT)
        j.push_back("LEFT");
      else if(side == BorderSides::RIGHT)
        j.push_back("RIGHT");
    }
  }

  template <typename tJson> void from_json(const tJson& j, BorderSides& out)
  {
    std::vector<BorderSides::Side> sides;
    for(const auto& s : j)
    {
      const auto value = s.template get<std::string>();
      if(value == "TOP")
        sides.emplace_back(BorderSides::TOP);
      else if(value == "BOTTOM")
        sides.emplace_back(BorderSides::BOTTOM);
      else if(value == "LEFT")
        sides.emplace_back(BorderSides::LEFT);
      else if(value == "RIGHT")
        sides.emplace_back(BorderSides::RIGHT);
      else
        throw std::invalid_argument("invalid BorderSide: " + value);
    }

    out.sides = std::move(sides);
  }

  template <typename tJson> void to_json(tJson& j, const Font& in)
  {
    j = tJson { { "size", in.size }, { "weight", Font::getWeightString(in.weight) }, { "baseName", in.baseName }, { "fallbackBaseNames", in.fallbackBaseNames } };
  }

  template <typename tJson> void from_json(const tJson& j, Font& out)
  {
    out = Font { };
    if(j.contains("size"))
      out.size = j.at("size").template get<int>();

    if(j.contains("weight"))
      out.weight = Detail::parseFontWeight(j.at("weight").template get<std::string>());

    if(j.contains("baseName"))
      out.baseName = j.at("baseName").template get<std::string>();

    if(j.contains("fallbackBaseNames"))
      out.fallbackBaseNames = j.at("fallbackBaseNames").template get<std::vector<std::string>>();
  }

  template <typename tJson> void to_json(tJson& j, const TextAlign& in)
  {
    if(in == TextAlign::CENTER())
      j = "CENTER";
    else if(in == TextAlign::LEFT())
      j = "LEFT";
    else if(in == TextAlign::RIGHT())
      j = "RIGHT";
    else
      j = static_cast<int>(in.it);
  }

  template <typename tJson> void from_json(const tJson& j, TextAlign& out)
  {
    out = Detail::parseTextAlign(j.template get<std::string>());
  }

  template <typename tJson> void to_json(tJson& j, const VerticalAlign& in)
  {
    if(in == VerticalAlign::TOP())
      j = "TOP";
    else if(in == VerticalAlign::CENTER())
      j = "CENTER";
    else if(in == VerticalAlign::BOTTOM())
      j = "BOTTOM";
    else
      j = static_cast<int>(in.it);
  }

  template <typename tJson> void from_json(const tJson& j, VerticalAlign& out)
  {
    out = Detail::parseVerticalAlign(j.template get<std::string>());
  }

  template <typename tJson> void to_json(tJson& j, const FlexAlign& in)
  {
    if(in == FlexAlign::CENTER())
      j = "CENTER";
    else if(in == FlexAlign::START())
      j = "START";
    else if(in == FlexAlign::END())
      j = "END";
    else if(in == FlexAlign::CENTER_START())
      j = "CENTER_START";
    else if(in == FlexAlign::CENTER_END())
      j = "CENTER_END";
    else if(in == FlexAlign::START_END())
      j = "START_END";
    else if(in == FlexAlign::START_CENTER())
      j = "START_CENTER";
    else if(in == FlexAlign::END_CENTER())
      j = "END_CENTER";
    else if(in == FlexAlign::END_START())
      j = "END_START";
    else if(in == FlexAlign::SPACE_EVENLY())
      j = "SPACE_EVENLY";
    else if(in == FlexAlign::SPACE_BETWEEEN())
      j = "SPACE_BETWEEN";
    else
      j = tJson { { "main", in.main }, { "cross", in.cross }, { "track_cross", in.track_cross } };
  }

  template <typename tJson> void from_json(const tJson& j, FlexAlign& out)
  {
    out = Detail::parseFlexAlign(j.template get<std::string>());
  }

  template <typename tJson> void to_json(tJson& j, const FlexFlow& in)
  {
    if(in == FlexFlow::HORIZONTAL())
      j = "HORIZONTAL";
    else if(in == FlexFlow::HORIZONTAL_REVERSE())
      j = "HORIZONTAL_REVERSE";
    else if(in == FlexFlow::HORIZONTAL_WRAP())
      j = "HORIZONTAL_WRAP";
    else if(in == FlexFlow::HORIZONTAL_WRAP_REVERSE())
      j = "HORIZONTAL_WRAP_REVERSE";
    else if(in == FlexFlow::VERTICAL())
      j = "VERTICAL";
    else if(in == FlexFlow::VERTICAL_REVERSE())
      j = "VERTICAL_REVERSE";
    else if(in == FlexFlow::VERTICAL_WRAP())
      j = "VERTICAL_WRAP";
    else if(in == FlexFlow::VERTICAL_WRAP_REVERSE())
      j = "VERTICAL_WRAP_REVERSE";
    else
      j = static_cast<int>(in.it);
  }

  template <typename tJson> void from_json(const tJson& j, FlexFlow& out)
  {
    out = Detail::parseFlexFlow(j.template get<std::string>());
  }

  template <typename tJson> void to_json(tJson& j, const FlexGap& in)
  {
    j = tJson { { "row", in.row }, { "column", in.column } };
  }

  template <typename tJson> void from_json(const tJson& j, FlexGap& out)
  {
    if(j.is_number_integer())
    {
      out = FlexGap { j.template get<int>(), j.template get<int>() };
      return;
    }

    if(j.is_object())
    {
      out = FlexGap { j.value("row", 0), j.value("column", 0) };
      return;
    }

    throw std::invalid_argument("FlexGap must be int or { row, column }");
  }

  template <typename tJson> void to_json(tJson& j, const Expand& in)
  {
    if(in == Expand::VERTICAL())
      j = "VERTICAL";
    else if(in == Expand::HORIZONTAL())
      j = "HORIZONTAL";
    else if(in == Expand::NONE())
      j = "NONE";
    else if(in == Expand::BOTH())
      j = "BOTH";
    else
      j = tJson { { "vertical", in.vertical }, { "horizontal", in.horizontal } };
  }

  template <typename tJson> void from_json(const tJson& j, Expand& out)
  {
    out = Detail::parseExpand(j.template get<std::string>());
  }

  template <typename tJson> void to_json(tJson& j, const Scrollable& in)
  {
    j = in.it == Scrollable::SCROLLABLE ? "SCROLL" : "NO_SCROLL";
  }

  template <typename tJson> void to_json(tJson& j, const RoundedCorner& in)
  {
    j = in.radius;
  }

  std::vector<std::unique_ptr<StyleSheet>> loadStyleSheetsFromFiles(const std::vector<std::filesystem::path>& files);
  std::vector<std::unique_ptr<StyleSheet>> loadStyleSheetsFromJsonFiles(const std::vector<std::filesystem::path>& files);
}
