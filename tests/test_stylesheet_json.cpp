#include "compose/modifiers/StyleSheetJson.h"

#include <catch2/catch_all.hpp>

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace
{
  std::filesystem::path writeTempStyleSheet(const std::string& content)
  {
    const auto timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const auto path = std::filesystem::temp_directory_path() / std::filesystem::path("compose-lvgl-stylesheet-" + std::to_string(timestamp) + ".json");
    std::ofstream out(path);
    REQUIRE(out.is_open());
    out << content;
    out.close();
    return path;
  }

  const Compose::StyleSheet* findChildByName(const Compose::StyleSheet& sheet, const std::string& name)
  {
    for(const auto& child : sheet.children)
    {
      if(child.name == name)
        return &child;
    }

    return nullptr;
  }
}

TEST_CASE("StyleSheetJson parses dot-prefixed class names without dot", "[StyleSheetJson]")
{
  const auto path = writeTempStyleSheet(R"json(
{
  ".root": {
    ".child": {
      "width": 10
    }
  }
}
)json");

  const auto removeFile = [&] { std::filesystem::remove(path); };
  const auto sheets = Compose::loadStyleSheetsFromJsonFiles({ path });
  removeFile();

  REQUIRE(sheets.size() == 1);
  REQUIRE(sheets[0].name == "root");
  REQUIRE(findChildByName(sheets[0], "child") != nullptr);
  REQUIRE(findChildByName(sheets[0], ".child") == nullptr);
}

TEST_CASE("StyleSheetJson preserves class order from file", "[StyleSheetJson]")
{
  const auto path = writeTempStyleSheet(R"json(
{
  ".root": {
    ".primary-color": {},
    ".fit-content": {},
    ".pages-area": {},
    ".background-color": {}
  }
}
)json");

  const auto removeFile = [&] { std::filesystem::remove(path); };
  const auto sheets = Compose::loadStyleSheetsFromJsonFiles({ path });
  removeFile();

  REQUIRE(sheets.size() == 1);
  REQUIRE(sheets[0].children.size() == 4);
  REQUIRE(sheets[0].children[0].name == "primary-color");
  REQUIRE(sheets[0].children[1].name == "fit-content");
  REQUIRE(sheets[0].children[2].name == "pages-area");
  REQUIRE(sheets[0].children[3].name == "background-color");
}

TEST_CASE("StyleSheetJson resolves @ assignments and &. merged classes", "[StyleSheetJson]")
{
  const auto path = writeTempStyleSheet(R"json(
{
  ".root": {
    "@yellow": "#fff514",
    ".background-color": {
      "&.yellow": {
        "background-color": "@yellow"
      },
      ".plain-child": {
        "width": 7
      }
    }
  }
}
)json");

  const auto removeFile = [&] { std::filesystem::remove(path); };
  const auto sheets = Compose::loadStyleSheetsFromJsonFiles({ path });
  removeFile();

  REQUIRE(sheets.size() == 1);
  const auto& root = sheets[0];
  REQUIRE(root.name == "root");

  const auto* backgroundColor = findChildByName(root, "background-color");
  REQUIRE(backgroundColor != nullptr);
  REQUIRE(findChildByName(*backgroundColor, "plain-child") != nullptr);

  const auto* merged = findChildByName(root, "background-color-yellow");
  REQUIRE(merged != nullptr);
  REQUIRE(findChildByName(*backgroundColor, "background-color-yellow") == nullptr);

  const auto color = std::get<std::optional<Compose::BackgroundColor>>(merged->styles);
  REQUIRE(color.has_value());
  REQUIRE(static_cast<Compose::Color>(color.value()) == Compose::Color::fromHEXString("#fff514"));
}

TEST_CASE("StyleSheetJson parses border-width and border-color", "[StyleSheetJson]")
{
  const auto path = writeTempStyleSheet(R"json(
{
  ".root": {
    ".outlined": {
      "border-width": 3,
      "border-color": "#112233"
    }
  }
}
)json");

  const auto removeFile = [&] { std::filesystem::remove(path); };
  const auto sheets = Compose::loadStyleSheetsFromJsonFiles({ path });
  removeFile();

  REQUIRE(sheets.size() == 1);
  const auto* outlined = findChildByName(sheets[0], "outlined");
  REQUIRE(outlined != nullptr);

  const auto borderWidth = std::get<std::optional<Compose::BorderWidth>>(outlined->styles);
  REQUIRE(borderWidth.has_value());
  REQUIRE(borderWidth->width == 3);

  const auto borderColor = std::get<std::optional<Compose::BorderColor>>(outlined->styles);
  REQUIRE(borderColor.has_value());
  REQUIRE(borderColor->color == Compose::Color::fromHEXString("#112233"));
}

TEST_CASE("StyleSheetJson logs unknown properties and ignores them as classes", "[StyleSheetJson]")
{
  const auto path = writeTempStyleSheet(R"json(
{
  ".root": {
    "unknown-property": {
      "width": 10
    }
  }
}
)json");

  std::stringstream err;
  auto* old = std::cerr.rdbuf(err.rdbuf());
  const auto sheets = Compose::loadStyleSheetsFromJsonFiles({ path });
  std::cerr.rdbuf(old);
  std::filesystem::remove(path);

  REQUIRE(sheets.size() == 1);
  REQUIRE(findChildByName(sheets[0], "unknown-property") == nullptr);
  REQUIRE(err.str().find("unknown property") != std::string::npos);
}
