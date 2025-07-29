#pragma once

namespace Compose
{
  struct FontSize
  {
    int it;

    bool operator==(const FontSize &o) const noexcept
    {
      return it == o.it;
    }
  };
}

namespace std
{
  template <> struct hash<Compose::FontSize>
  {
    std::size_t operator()(const Compose::FontSize &sz) const noexcept
    {
      return sz.it;
    }
  };
}