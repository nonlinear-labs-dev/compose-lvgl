#pragma once
#include <string>
#include <vector>

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

    std::string baseName = "AlteHaas";
    std::vector<std::string> fallbackBaseNames { "FontAwesome" };

    bool operator==(const Font &o) const noexcept
    {
      return size == o.size && baseName == o.baseName && weight == o.weight && fallbackBaseNames == o.fallbackBaseNames;
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
      const std::hash<std::string> stringHasher {};
      const std::hash<int> intHasher {};
      std::size_t hash = intHasher(sz.size);
      hash ^= intHasher(static_cast<int>(sz.weight)) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
      hash ^= stringHasher(sz.baseName) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
      for(const auto &fallbackName : sz.fallbackBaseNames)
        hash ^= stringHasher(fallbackName) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
      return hash;
    }
  };
}
