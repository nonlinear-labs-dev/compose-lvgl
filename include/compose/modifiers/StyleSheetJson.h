#pragma once

#include <nlohmann/json.hpp>
#include <vector>
#include <filesystem>

namespace Compose
{
  struct StyleSheet;
  struct RoundedCorner;
  struct Scrollable;
  struct Expand;
  struct FlexFlow;
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

  void to_json(nlohmann::json& j, const Color& in);
  void from_json(const nlohmann::json& j, Color& out);

  void to_json(nlohmann::json& j, const Width& in);
  void from_json(const nlohmann::json& j, Width& out);

  void to_json(nlohmann::json& j, const Height& in);
  void from_json(const nlohmann::json& j, Height& out);

  void to_json(nlohmann::json& j, const Margin& in);
  void from_json(const nlohmann::json& j, Margin& out);

  void to_json(nlohmann::json& j, const MarginLeft& in);
  void to_json(nlohmann::json& j, const MarginRight& in);
  void to_json(nlohmann::json& j, const MarginTop& in);
  void to_json(nlohmann::json& j, const MarginBottom& in);

  void to_json(nlohmann::json& j, const Padding& in);
  void from_json(const nlohmann::json& j, Padding& out);

  void to_json(nlohmann::json& j, const Border& in);
  void from_json(const nlohmann::json& j, Border& out);

  void to_json(nlohmann::json& j, const BorderWidth& in);
  void to_json(nlohmann::json& j, const BorderColor& in);

  void to_json(nlohmann::json& j, const BorderSides& in);
  void from_json(const nlohmann::json& j, BorderSides& out);

  void to_json(nlohmann::json& j, const Font& in);
  void from_json(const nlohmann::json& j, Font& out);

  void to_json(nlohmann::json& j, const TextAlign& in);
  void from_json(const nlohmann::json& j, TextAlign& out);

  void to_json(nlohmann::json& j, const VerticalAlign& in);
  void from_json(const nlohmann::json& j, VerticalAlign& out);

  void to_json(nlohmann::json& j, const FlexAlign& in);
  void from_json(const nlohmann::json& j, FlexAlign& out);

  void to_json(nlohmann::json& j, const FlexFlow& in);
  void from_json(const nlohmann::json& j, FlexFlow& out);

  void to_json(nlohmann::json& j, const Expand& in);
  void from_json(const nlohmann::json& j, Expand& out);

  void to_json(nlohmann::json& j, const Scrollable& in);
  void from_json(const nlohmann::json& j, Scrollable& out);

  void to_json(nlohmann::json& j, const RoundedCorner& in);

  std::vector<std::unique_ptr<StyleSheet>> loadStyleSheetsFromFiles(const std::vector<std::filesystem::path>& files);
  std::vector<std::unique_ptr<StyleSheet>> loadStyleSheetsFromJsonFiles(const std::vector<std::filesystem::path>& files);
}
