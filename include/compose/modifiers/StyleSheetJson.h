#pragma once

#include <vector>

#include "compose/modifiers/StyleSheets.h"
#include "tools/json.h"

#include <filesystem>

namespace Compose
{
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

  void to_json(nlohmann::json& j, const Scrollable& in);
  void from_json(const nlohmann::json& j, Scrollable& out);

  void to_json(nlohmann::json& j, const RoundedCorner& in);

  std::vector<StyleSheet> loadStyleSheetsFromJsonFiles(const std::vector<std::filesystem::path>& files);
}
