#include "compose/modifiers/StyleSheetJson.h"

#include "compose/modifiers/Border.h"
#include "compose/modifiers/Font.h"
#include "compose/modifiers/Modifiers.h"
#include "compose/modifiers/Padding.h"
#include "compose/modifiers/RoundedCorner.h"
#include "compose/modifiers/StyleSheets.h"
#include <nlohmann/json.hpp>
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
    addProperty(ret, "flex-gap", std::get<std::optional<FlexGap>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "expand", std::get<std::optional<Expand>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "width", std::get<std::optional<Width>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "height", std::get<std::optional<Height>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "margin", std::get<std::optional<Margin>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "margin-left", std::get<std::optional<MarginLeft>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "margin-right", std::get<std::optional<MarginRight>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "margin-top", std::get<std::optional<MarginTop>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "margin-bottom", std::get<std::optional<MarginBottom>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "padding", std::get<std::optional<Padding>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "x", std::get<std::optional<PositionX>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "y", std::get<std::optional<PositionY>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "border", std::get<std::optional<Border>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "border-width", std::get<std::optional<BorderWidth>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "border-color", std::get<std::optional<BorderColor>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "border-sides", std::get<std::optional<BorderSides>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "rounded-corner", std::get<std::optional<RoundedCorner>>(properties), onlyNonNullOptProperties);
    addProperty(ret, "scrollable", std::get<std::optional<Scrollable>>(properties), onlyNonNullOptProperties);

    return ret.dump();
  }

  using JsonModifier = std::variant<BackgroundColor, PrimaryColor, Font, TextAlign, VerticalAlign, FlexAlign, FlexFlow, FlexGap, Expand, Width, Height, Margin, MarginLeft,
                                    MarginRight, MarginTop, MarginBottom, Padding, PositionX, PositionY, Border, BorderWidth, BorderColor, BorderSides, RoundedCorner, Scrollable>;

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

    if(key == "flex-gap")
      return value.get<FlexGap>();

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

    if(key == "x")
      return PositionX { value.get<int>() };

    if(key == "y")
      return PositionY { value.get<int>() };

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
