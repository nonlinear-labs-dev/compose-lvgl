#include "compose/modifiers/StyleSheetJson.h"

#include "compose/modifiers/Border.h"
#include "compose/modifiers/Padding.h"
#include "compose/modifiers/RoundedCorner.h"
#include "tools/json.h"

#include <fstream>
#include <optional>
#include <stdexcept>
#include <variant>

namespace
{
  Compose::Font::FontWeight parseFontWeight(const std::string& weight)
  {
    if(weight == "Black")
      return Compose::Font::FontWeight::Black;
    if(weight == "ExtraBold")
      return Compose::Font::FontWeight::ExtraBold;
    if(weight == "Bold")
      return Compose::Font::FontWeight::Bold;
    if(weight == "SemiBold")
      return Compose::Font::FontWeight::SemiBold;
    if(weight == "Medium")
      return Compose::Font::FontWeight::Medium;
    if(weight == "Light")
      return Compose::Font::FontWeight::Light;

    return Compose::Font::FontWeight::Regular;
  }

  Compose::TextAlign parseTextAlign(const std::string& value)
  {
    if(value == "CENTER")
      return Compose::TextAlign::CENTER();
    if(value == "LEFT")
      return Compose::TextAlign::LEFT();
    if(value == "RIGHT")
      return Compose::TextAlign::RIGHT();

    throw std::invalid_argument("invalid TextAlign: " + value);
  }

  Compose::VerticalAlign parseVerticalAlign(const std::string& value)
  {
    if(value == "TOP")
      return Compose::VerticalAlign::TOP();
    if(value == "CENTER")
      return Compose::VerticalAlign::CENTER();
    if(value == "BOTTOM")
      return Compose::VerticalAlign::BOTTOM();

    throw std::invalid_argument("invalid VerticalAlign: " + value);
  }

  Compose::FlexAlign parseFlexAlign(const std::string& value)
  {
    if(value == "CENTER")
      return Compose::FlexAlign::CENTER();
    if(value == "START")
      return Compose::FlexAlign::START();
    if(value == "END")
      return Compose::FlexAlign::END();
    if(value == "CENTER_START")
      return Compose::FlexAlign::CENTER_START();
    if(value == "CENTER_END")
      return Compose::FlexAlign::CENTER_END();
    if(value == "START_END")
      return Compose::FlexAlign::START_END();
    if(value == "START_CENTER")
      return Compose::FlexAlign::START_CENTER();
    if(value == "END_CENTER")
      return Compose::FlexAlign::END_CENTER();
    if(value == "END_START")
      return Compose::FlexAlign::END_START();
    if(value == "SPACE_EVENLY")
      return Compose::FlexAlign::SPACE_EVENLY();

    throw std::invalid_argument("invalid FlexAlign: " + value);
  }
}

namespace Compose
{
  void to_json(nlohmann::json& j, const Color& in)
  {
    j = "#" + in.toHEXString();
  }

  void from_json(const nlohmann::json& j, Color& out)
  {
    out = Color::fromHEXString(j.get<std::string>());
  }

  void to_json(nlohmann::json& j, const Width& in)
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

  void from_json(const nlohmann::json& j, Width& out)
  {
    if(j.is_string())
    {
      const auto v = j.get<std::string>();
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

    out = Width { j.get<int>() };
  }

  void to_json(nlohmann::json& j, const Height& in)
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

  void from_json(const nlohmann::json& j, Height& out)
  {
    if(j.is_string())
    {
      const auto v = j.get<std::string>();
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

    out = Height { j.get<int>() };
  }

  void to_json(nlohmann::json& j, const Margin& in)
  {
    j = nlohmann::json::array({ in.top, in.bottom, in.left, in.right });
  }

  void to_json(nlohmann::json& j, const MarginLeft& in)
  {
    j = in.margin;
  }

  void to_json(nlohmann::json& j, const MarginRight& in)
  {
    j = in.margin;
  }

  void to_json(nlohmann::json& j, const MarginTop& in)
  {
    j = in.margin;
  }

  void to_json(nlohmann::json& j, const MarginBottom& in)
  {
    j = in.margin;
  }

  void from_json(const nlohmann::json& j, Margin& out)
  {
    if(j.is_number_integer())
    {
      out = Margin { j.get<int>() };
      return;
    }

    if(j.is_array() && j.size() == 4)
    {
      out = Margin { j.at(0).get<int>(), j.at(1).get<int>(), j.at(2).get<int>(), j.at(3).get<int>() };
      return;
    }

    throw std::invalid_argument("Margin must be int or [top, bottom, left, right]");
  }

  void to_json(nlohmann::json& j, const Padding& in)
  {
    j = nlohmann::json::array({ in.top, in.bottom, in.left, in.right });
  }

  void from_json(const nlohmann::json& j, Padding& out)
  {
    if(j.is_number_integer())
    {
      out = Padding { j.get<int>() };
      return;
    }

    if(j.is_array() && j.size() == 4)
    {
      out = Padding { j.at(0).get<int>(), j.at(1).get<int>(), j.at(2).get<int>(), j.at(3).get<int>() };
      return;
    }

    throw std::invalid_argument("Padding must be int or [top, bottom, left, right]");
  }

  void to_json(nlohmann::json& j, const Border& in)
  {
    j = nlohmann::json { { "width", in.width }, { "color", in.color } };
  }

  void from_json(const nlohmann::json& j, Border& out)
  {
    out = Border { .width = j.at("width").get<int>(), .color = j.at("color").get<Color>() };
  }

  void to_json(nlohmann::json& j, const BorderSides& in)
  {
    j = nlohmann::json::array();
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

  void from_json(const nlohmann::json& j, BorderSides& out)
  {
    std::vector<BorderSides::Side> sides;
    for(const auto& s : j)
    {
      const auto value = s.get<std::string>();
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

  void to_json(nlohmann::json& j, const Font& in)
  {
    j = nlohmann::json { { "size", in.size }, { "weight", Font::getWeightString(in.weight) }, { "baseName", in.baseName }, { "fallbackBaseNames", in.fallbackBaseNames } };
  }

  void from_json(const nlohmann::json& j, Font& out)
  {
    out = Font {};
    out.size = j.at("size").get<int>();

    if(j.contains("weight"))
      out.weight = parseFontWeight(j.at("weight").get<std::string>());

    if(j.contains("baseName"))
      out.baseName = j.at("baseName").get<std::string>();

    if(j.contains("fallbackBaseNames"))
      out.fallbackBaseNames = j.at("fallbackBaseNames").get<std::vector<std::string>>();
  }

  void to_json(nlohmann::json& j, const TextAlign& in)
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

  void from_json(const nlohmann::json& j, TextAlign& out)
  {
    out = parseTextAlign(j.get<std::string>());
  }

  void to_json(nlohmann::json& j, const VerticalAlign& in)
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

  void from_json(const nlohmann::json& j, VerticalAlign& out)
  {
    out = parseVerticalAlign(j.get<std::string>());
  }

  void to_json(nlohmann::json& j, const FlexAlign& in)
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
    else
      j = nlohmann::json { { "main", in.main }, { "cross", in.cross }, { "track_cross", in.track_cross } };
  }

  void from_json(const nlohmann::json& j, FlexAlign& out)
  {
    out = parseFlexAlign(j.get<std::string>());
  }

  void to_json(nlohmann::json& j, const Scrollable& in)
  {
    j = in.it == Scrollable::SCROLLABLE ? "SCROLL" : "NO_SCROLL";
  }

  void to_json(nlohmann::json& j, const RoundedCorner& in)
  {
    j = in.radius;
  }

  void from_json(const nlohmann::json& j, Scrollable& out)
  {
    if(j.is_boolean())
    {
      out = Scrollable::fromBool(j.get<bool>());
      return;
    }

    const auto value = j.get<std::string>();
    if(value == "NO_SCROLL")
    {
      out = Scrollable::NO_SCROLL();
      return;
    }

    if(value == "SCROLL")
    {
      out = Scrollable::SCROLL();
      return;
    }

    throw std::invalid_argument("invalid Scrollable value: " + value);
  }
}

namespace Compose
{
  template <typename T> void addProperty(nlohmann::json& json, const char* key, const std::optional<T>& value, bool onlyNonNullOptProperties)
  {
    if(value)
      json[key] = value.value();
    else if(!onlyNonNullOptProperties)
      json[key] = nullptr;
  }

  std::string Style::dump(bool onlyNonNullOptProperties) const
  {
    auto ret = nlohmann::json::object();
    addProperty(ret, "background-color", std::get<std::optional<BackgroundColor>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "primary-color", std::get<std::optional<PrimaryColor>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "font", std::get<std::optional<Font>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "text-align", std::get<std::optional<TextAlign>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "vertical-align", std::get<std::optional<VerticalAlign>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "flex-align", std::get<std::optional<FlexAlign>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "width", std::get<std::optional<Width>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "height", std::get<std::optional<Height>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "margin", std::get<std::optional<Margin>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "margin-left", std::get<std::optional<MarginLeft>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "margin-right", std::get<std::optional<MarginRight>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "margin-top", std::get<std::optional<MarginTop>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "margin-bottom", std::get<std::optional<MarginBottom>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "padding", std::get<std::optional<Padding>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "border", std::get<std::optional<Border>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "border-sides", std::get<std::optional<BorderSides>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "rounded-corner", std::get<std::optional<RoundedCorner>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "scrollable", std::get<std::optional<Scrollable>>(properties), onlyNonNullOptProperties);

    return ret.dump();
  }

  using JsonModifier = std::variant<BackgroundColor, PrimaryColor, Font, TextAlign, VerticalAlign, FlexAlign, Width, Height, Margin, MarginLeft, MarginRight, MarginTop, MarginBottom,
                                    Padding, Border, BorderSides, RoundedCorner, Scrollable>;

  static std::optional<JsonModifier> parseModifier(const std::string& key, const nlohmann::json& value)
  {
    if(key == "background-color")
      return BackgroundColor { value.get<Color>() };

    if(key == "primary-color")
      return PrimaryColor { value.get<Color>() };

    if(key == "font")
      return value.get<Font>();

    if(key == "text-align")
      return value.get<TextAlign>();

    if(key == "vertical-align")
      return value.get<VerticalAlign>();

    if(key == "flex-align")
      return value.get<FlexAlign>();

    if(key == "width")
      return value.get<Width>();

    if(key == "height")
      return value.get<Height>();

    if(key == "margin")
      return value.get<Margin>();

    if(key == "margin-left")
      return MarginLeft { value.get<int>() };

    if(key == "margin-right")
      return MarginRight { value.get<int>() };

    if(key == "margin-top")
      return MarginTop { value.get<int>() };

    if(key == "margin-bottom")
      return MarginBottom { value.get<int>() };

    if(key == "padding")
      return value.get<Padding>();

    if(key == "border")
      return value.get<Border>();

    if(key == "border-sides")
      return value.get<BorderSides>();

    if(key == "rounded-corner")
      return RoundedCorner { value.get<int>() };

    if(key == "scrollable")
      return value.get<Scrollable>();

    return {};
  }

  static StyleSheet parseStyleSheet(const std::string& name, const nlohmann::json& definition)
  {
    if(!definition.is_object())
      throw std::invalid_argument("Style definition for '" + name + "' must be an object");

    StyleSheet ret { name };

    for(const auto& [key, value] : definition.items())
    {
      if(auto modifier = parseModifier(key, value))
      {
        std::visit([&](auto&& m) { ret.apply(std::forward<decltype(m)>(m)); }, modifier.value());
      }
      else
      {
        ret.apply(parseStyleSheet(key, value));
      }
    }

    return ret;
  }

  std::vector<StyleSheet> loadStyleSheetsFromJsonFiles(const std::vector<std::filesystem::path>& files)
  {
    std::vector<StyleSheet> ret;

    for(const auto& file : files)
    {
      std::ifstream stream(file);
      if(!stream.is_open())
        throw std::invalid_argument("Could not open style sheet file: " + file.string());

      nlohmann::json root;
      stream >> root;

      if(!root.is_object())
        throw std::invalid_argument("Style sheet root must be an object");

      for(const auto& [styleName, styleDef] : root.items())
      {
        ret.emplace_back(parseStyleSheet(styleName, styleDef));
      }
    }

    return ret;
  }
}
