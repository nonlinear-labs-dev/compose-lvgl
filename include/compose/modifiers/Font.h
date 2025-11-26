#pragma once
#include <string>

namespace Compose
{
  struct Font
  {
    int size = 12;

    enum class FontWeight
    {
      Black,
      ExtraBold,
      Bold,
      SemiBold,
      Medium,
      Regular,
      Light,
    } weight
        = FontWeight::Regular;

    std::string baseName = "Nonlegible-Rubik";

    bool operator==(const Font &o) const noexcept
    {
      return size == o.size && baseName == o.baseName && weight == o.weight;
    }

    static std::string getWeightString(FontWeight weight)
    {
      switch(weight)
      {
        case FontWeight::Black:
          return "Black";
        case FontWeight::Bold:
          return "Bold";
        case FontWeight::ExtraBold:
          return "ExtraBold";
        case FontWeight::Light:
          return "Light";
        case FontWeight::Medium:
          return "Medium";
        case FontWeight::SemiBold:
          return "SemiBold";
        case FontWeight::Regular:
        default:
          return "Regular";
      }
    }
  };
}

namespace std
{
  template <> struct hash<Compose::Font>
  {
    std::size_t operator()(const Compose::Font &sz) const noexcept
    {
      constexpr std::hash<std::string> h {};
      return sz.size * 1000 + h(sz.baseName);
    }
  };
}