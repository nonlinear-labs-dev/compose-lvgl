#include "compose/FreeTypeFont.h"
#include "compose/modifiers/StyleSheetJson.h"

#include <catch2/catch_all.hpp>

#include <filesystem>
#include <set>

namespace
{
  constexpr auto c_fontSize = 40;

  std::string testFontPath()
  {
    return std::filesystem::path(__FILE__).parent_path() / "manual" / "Nonlegible-Rubik-Regular.ttf";
  }

  std::set<int> digitWidths(const Compose::Font::Digits digits)
  {
    const Compose::FreeTypeFont font({ testFontPath() }, c_fontSize, digits);
    std::set<int> widths;
    for(const auto digit : Glib::ustring("0123456789"))
      widths.insert(font.getStringWidth(Glib::ustring(1, digit)));
    return widths;
  }
}

TEST_CASE("tabular digits all have the same width")
{
  REQUIRE(digitWidths(Compose::Font::Digits::Tabular).size() == 1);
}

TEST_CASE("proportional digits keep their individual widths")
{
  REQUIRE(digitWidths(Compose::Font::Digits::Proportional).size() > 1);
}

TEST_CASE("tabular digits do not change other characters")
{
  const Compose::FreeTypeFont proportional({ testFontPath() }, c_fontSize, Compose::Font::Digits::Proportional);
  const Compose::FreeTypeFont tabular({ testFontPath() }, c_fontSize, Compose::Font::Digits::Tabular);
  REQUIRE(tabular.getStringWidth("Hz -.") == proportional.getStringWidth("Hz -."));
}

TEST_CASE("fonts differing only in digits are distinct")
{
  const Compose::Font proportional {};
  const Compose::Font tabular { .digits = Compose::Font::Digits::Tabular };
  REQUIRE_FALSE(proportional == tabular);
  REQUIRE(std::hash<Compose::Font> {}(proportional) != std::hash<Compose::Font> {}(tabular));
}

TEST_CASE("font digits survive a json round trip")
{
  const nlohmann::json json = Compose::Font { .digits = Compose::Font::Digits::Tabular };
  REQUIRE(json.get<Compose::Font>().digits == Compose::Font::Digits::Tabular);
}
