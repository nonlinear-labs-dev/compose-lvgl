#include "compose/modifiers/StyleSheetJson.h"

#include "compose/modifiers/Border.h"
#include "compose/modifiers/Font.h"
#include "compose/modifiers/Modifiers.h"
#include "compose/modifiers/Padding.h"
#include "compose/modifiers/RoundedCorner.h"
#include "compose/modifiers/StyleSheets.h"
#include "tools/json.h"
#include <yaml-cpp/yaml.h>

#include <charconv>
#include <cctype>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
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
    if(value == "SPACE_BETWEEN")
      return Compose::FlexAlign::SPACE_BETWEEEN();

    throw std::invalid_argument("invalid FlexAlign: " + value);
  }

  Compose::FlexFlow parseFlexFlow(const std::string& value)
  {
    if(value == "HORIZONTAL")
      return Compose::FlexFlow::HORIZONTAL();
    if(value == "HORIZONTAL_REVERSE")
      return Compose::FlexFlow::HORIZONTAL_REVERSE();
    if(value == "HORIZONTAL_WRAP")
      return Compose::FlexFlow::HORIZONTAL_WRAP();
    if(value == "HORIZONTAL_WRAP_REVERSE")
      return Compose::FlexFlow::HORIZONTAL_WRAP_REVERSE();
    if(value == "VERTICAL")
      return Compose::FlexFlow::VERTICAL();
    if(value == "VERTICAL_REVERSE")
      return Compose::FlexFlow::VERTICAL_REVERSE();
    if(value == "VERTICAL_WRAP")
      return Compose::FlexFlow::VERTICAL_WRAP();
    if(value == "VERTICAL_WRAP_REVERSE")
      return Compose::FlexFlow::VERTICAL_WRAP_REVERSE();

    throw std::invalid_argument("invalid FlexFlow: " + value);
  }

  Compose::Expand parseExpand(const std::string& value)
  {
    if(value == "VERTICAL")
      return Compose::Expand::VERTICAL();
    if(value == "HORIZONTAL")
      return Compose::Expand::HORIZONTAL();
    if(value == "NONE")
      return Compose::Expand::NONE();
    if(value == "BOTH")
      return Compose::Expand::BOTH();

    throw std::invalid_argument("invalid Expand: " + value);
  }
}

namespace Compose
{
  using OrderedJson = nlohmann::ordered_json;
  using ValueAssignments = std::unordered_map<std::string, OrderedJson>;

  static std::string toLowerCase(std::string input)
  {
    for(auto& c : input)
      c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return input;
  }

  static OrderedJson parseYamlScalarValue(const std::string& scalar)
  {
    const auto lowered = toLowerCase(scalar);
    if(lowered == "null" || lowered == "~")
      return nullptr;

    if(lowered == "true" || lowered == "yes" || lowered == "on")
      return true;

    if(lowered == "false" || lowered == "no" || lowered == "off")
      return false;

    long long intValue = 0;
    if(auto [ptr, ec] = std::from_chars(scalar.data(), scalar.data() + scalar.size(), intValue); ec == std::errc() && ptr == scalar.data() + scalar.size())
      return intValue;

    if(scalar.find_first_of(".eE") != std::string::npos)
    {
      double floatValue = 0.0;
      if(auto [ptr, ec] = std::from_chars(scalar.data(), scalar.data() + scalar.size(), floatValue); ec == std::errc() && ptr == scalar.data() + scalar.size())
        return floatValue;
    }

    return scalar;
  }

  static OrderedJson yamlNodeToOrderedJson(const YAML::Node& node)
  {
    if(!node.IsDefined() || node.IsNull())
      return nullptr;

    if(node.IsScalar())
      return parseYamlScalarValue(node.Scalar());

    if(node.IsSequence())
    {
      auto json = OrderedJson::array();
      for(const auto& child : node)
        json.emplace_back(yamlNodeToOrderedJson(child));
      return json;
    }

    if(node.IsMap())
    {
      auto json = OrderedJson::object();
      for(const auto& entry : node)
      {
        if(!entry.first.IsScalar())
          throw std::invalid_argument("YAML style sheet keys must be scalar values");

        json[entry.first.as<std::string>()] = yamlNodeToOrderedJson(entry.second);
      }
      return json;
    }

    throw std::invalid_argument("Unsupported YAML node type in style sheet");
  }

  static OrderedJson loadStyleSheetRootFromFile(const std::filesystem::path& file)
  {
    std::ifstream stream(file);
    if(!stream.is_open())
      throw std::invalid_argument("Could not open style sheet file: " + file.string());

    auto extension = toLowerCase(file.extension().string());
    if(extension == ".yaml" || extension == ".yml")
    {
      auto rootNode = YAML::LoadFile(file.string());
      auto root = yamlNodeToOrderedJson(rootNode);
      if(root.is_null())
        root = OrderedJson::object();
      return root;
    }

    OrderedJson root;
    stream >> root;
    return root;
  }

  static bool isClassName(const std::string& key)
  {
    return !key.empty() && key.front() == '.';
  }

  static bool isValueAssignment(const std::string& key)
  {
    return !key.empty() && key.front() == '@';
  }

  static bool isMergedClassName(const std::string& key)
  {
    return key.size() > 2 && key[0] == '&' && key[1] == '.';
  }

  static std::string removePrefix(const std::string& key)
  {
    return key.substr(1);
  }

  static void logStyleSheetError(const std::string& styleName, const std::string& key, const std::string& reason)
  {
    std::cerr << "StyleSheetJson error in '" << styleName << "' for key '" << key << "': " << reason << std::endl;
  }

  static OrderedJson resolveAssignedValue(const OrderedJson& value, const ValueAssignments& assignments, std::unordered_set<std::string>& resolutionPath)
  {
    if(value.is_string())
    {
      const auto asString = value.get<std::string>();
      if(isValueAssignment(asString))
      {
        const auto assignmentName = removePrefix(asString);
        if(const auto it = assignments.find(assignmentName); it != assignments.end())
        {
          if(resolutionPath.contains(assignmentName))
            throw std::invalid_argument("cyclic value assignment detected for @" + assignmentName);

          resolutionPath.insert(assignmentName);
          auto resolved = resolveAssignedValue(it->second, assignments, resolutionPath);
          resolutionPath.erase(assignmentName);
          return resolved;
        }
      }
    }

    if(value.is_object())
    {
      auto resolved = value;
      for(auto& [nestedKey, nestedValue] : resolved.items())
        nestedValue = resolveAssignedValue(nestedValue, assignments, resolutionPath);
      return resolved;
    }

    if(value.is_array())
    {
      auto resolved = value;
      for(auto& nestedValue : resolved)
        nestedValue = resolveAssignedValue(nestedValue, assignments, resolutionPath);
      return resolved;
    }

    return value;
  }

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

  void to_json(nlohmann::json& j, const BorderWidth& in)
  {
    j = in.width;
  }

  void to_json(nlohmann::json& j, const BorderColor& in)
  {
    j = in.color;
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
    out = Font { };
    if(j.contains("size"))
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
    else if(in == FlexAlign::SPACE_BETWEEEN())
      j = "SPACE_BETWEEN";
    else
      j = nlohmann::json { { "main", in.main }, { "cross", in.cross }, { "track_cross", in.track_cross } };
  }

  void from_json(const nlohmann::json& j, FlexAlign& out)
  {
    out = parseFlexAlign(j.get<std::string>());
  }

  void to_json(nlohmann::json& j, const FlexFlow& in)
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

  void from_json(const nlohmann::json& j, FlexFlow& out)
  {
    out = parseFlexFlow(j.get<std::string>());
  }

  void to_json(nlohmann::json& j, const Expand& in)
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
      j = nlohmann::json { { "vertical", in.vertical }, { "horizontal", in.horizontal } };
  }

  void from_json(const nlohmann::json& j, Expand& out)
  {
    out = parseExpand(j.get<std::string>());
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
    addProperty(ret, "flex-flow", std::get<std::optional<FlexFlow>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "expand", std::get<std::optional<Expand>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "width", std::get<std::optional<Width>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "height", std::get<std::optional<Height>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "margin", std::get<std::optional<Margin>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "margin-left", std::get<std::optional<MarginLeft>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "margin-right", std::get<std::optional<MarginRight>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "margin-top", std::get<std::optional<MarginTop>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "margin-bottom", std::get<std::optional<MarginBottom>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "padding", std::get<std::optional<Padding>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "border", std::get<std::optional<Border>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "border-width", std::get<std::optional<BorderWidth>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "border-color", std::get<std::optional<BorderColor>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "border-sides", std::get<std::optional<BorderSides>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "rounded-corner", std::get<std::optional<RoundedCorner>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "scrollable", std::get<std::optional<Scrollable>>(properties), onlyNonNullOptProperties);

    return ret.dump();
  }

  using JsonModifier = std::variant<BackgroundColor, PrimaryColor, Font, TextAlign, VerticalAlign, FlexAlign, FlexFlow, Expand, Width, Height, Margin, MarginLeft, MarginRight,
                                    MarginTop, MarginBottom, Padding, Border, BorderWidth, BorderColor, BorderSides, RoundedCorner, Scrollable>;

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

    if(key == "flex-flow")
      return value.get<FlexFlow>();

    if(key == "expand")
      return value.get<Expand>();

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

    if(key == "border-width")
      return BorderWidth { value.get<int>() };

    if(key == "border-color")
      return BorderColor { value.get<Color>() };

    if(key == "border-sides")
      return value.get<BorderSides>();

    if(key == "rounded-corner")
      return RoundedCorner { value.get<int>() };

    if(key == "scrollable")
      return value.get<Scrollable>();

    return { };
  }

  static std::unique_ptr<StyleSheet> parseStyleSheet(const std::string& name, const OrderedJson& definition, const ValueAssignments& inheritedAssignments = { },
                                                     std::vector<std::unique_ptr<StyleSheet>>* sameContextSheets = nullptr)
  {
    if(!definition.is_object())
      throw std::invalid_argument("Style definition for '" + name + "' must be an object");

    std::unique_ptr<StyleSheet> ret = std::make_unique<StyleSheet>(name);
    auto assignments = inheritedAssignments;

    for(const auto& [key, value] : definition.items())
    {
      if(isValueAssignment(key))
      {
        const auto assignmentName = removePrefix(key);
        if(assignmentName.empty())
          logStyleSheetError(name, key, "value assignment name cannot be empty");
        else
        {
          assignments[assignmentName] = value;
          std::unordered_set<std::string> resolutionPath;
          ret->vars[assignmentName] = nlohmann::json(resolveAssignedValue(value, assignments, resolutionPath));
        }
      }
    }

    for(const auto& [key, value] : definition.items())
    {
      if(isMergedClassName(key))
      {
        const auto mergedClassName = removePrefix(key.substr(1));
        if(mergedClassName.empty())
          logStyleSheetError(name, key, "class name cannot be empty");
        else if(sameContextSheets)
          sameContextSheets->emplace_back(parseStyleSheet(name + "-" + mergedClassName, value, assignments, sameContextSheets));
        else
          logStyleSheetError(name, key, "cannot resolve merged class without context");
      }
      else if(!isValueAssignment(key) && isClassName(key))
      {
        const auto className = removePrefix(key);
        if(className.empty())
          logStyleSheetError(name, key, "class name cannot be empty");
        else
          ret->apply(parseStyleSheet(className, value, assignments, &ret->children));
      }
      else if(!isValueAssignment(key))
      {
        std::unordered_set<std::string> resolutionPath;
        const auto resolvedValue = resolveAssignedValue(value, assignments, resolutionPath);
        if(auto modifier = parseModifier(key, nlohmann::json(resolvedValue)))
          std::visit([&](auto&& m) { ret->apply(std::forward<decltype(m)>(m)); }, modifier.value());
        else
          ret->vars[key] = nlohmann::json(resolvedValue);
      }
    }

    return ret;
  }

  std::vector<std::unique_ptr<StyleSheet>> loadStyleSheetsFromFiles(const std::vector<std::filesystem::path>& files)
  {
    std::vector<std::unique_ptr<StyleSheet>> ret;

    try
    {
      OrderedJson mergedRoot = OrderedJson::object();

      for(const auto& file : files)
      {
        auto root = loadStyleSheetRootFromFile(file);

        if(!root.is_object())
          throw std::invalid_argument("Style sheet root must be an object");

        mergedRoot.update(root, true);
      }

      for(const auto& [styleName, styleDef] : mergedRoot.items())
      {
        if(styleName == ".")
          logStyleSheetError("root", styleName, "class name cannot be empty");
        else if(isClassName(styleName))
          ret.emplace_back(parseStyleSheet(removePrefix(styleName), styleDef, { }, &ret));
        else
          ret.emplace_back(parseStyleSheet(styleName, styleDef, { }, &ret));
      }
    }
    catch(std::exception& e)
    {
      logStyleSheetError("root", "", "Error loading style sheets: " + std::string(e.what()));
      return { };
    }

    return ret;
  }

  std::vector<std::unique_ptr<StyleSheet>> loadStyleSheetsFromJsonFiles(const std::vector<std::filesystem::path>& files)
  {
    return loadStyleSheetsFromFiles(files);
  }
}
