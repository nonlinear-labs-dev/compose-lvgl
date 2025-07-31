#pragma once

namespace Compose
{
  struct Font
  {
    int size = 12;

    enum class FontWeight
    {
      Black,
      Bold,
      ExtraBold,
      Light,
      Medium,
      Regular,
      SemiBold
    } weight
        = FontWeight::ExtraBold;

    bool operator==(const Font &o) const noexcept
    {
      return size == o.size && weight == o.weight;
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
      return sz.size * 1000 + static_cast<int>(sz.weight);
    }
  };
}