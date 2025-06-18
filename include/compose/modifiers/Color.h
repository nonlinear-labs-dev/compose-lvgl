#pragma once
#include <concepts>
#include <cstdint>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <string>
#include <concepts>
#include <format>

namespace Compose
{
  template <typename T>
  concept IntegralOrChar = std::integral<T> || std::same_as<T, char>;

  template <typename T>
  concept Floating = std::floating_point<T>;

  struct Color
  {
    using tColorValueType = unsigned char;
    constexpr static tColorValueType c_maxValue = 255;

    tColorValueType r = 0;
    tColorValueType g = 0;
    tColorValueType b = 0;
    float a = 1;

    constexpr auto static maxTFixed = 255;
    constexpr static tColorValueType maxValue = maxTFixed;

    template <IntegralOrChar tFixed>
    constexpr Color(tFixed _r, tFixed _g, tFixed _b, float _a = 1.0)
        : r(_r)
        , g(_g)
        , b(_b)
        , a(_a)
    {
    }

    Color() = default;
    Color(Color&& c) = default;
    Color(const Color& c) = default;
    Color& operator=(const Color&) = default;

    [[nodiscard]] std::string toHEXString() const
    {
      std::stringstream ss;
      ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(r) << std::setw(2)
         << static_cast<int>(g) << std::setw(2) << static_cast<int>(b) << std::setw(2)
         << static_cast<int>(a * maxTFixed);
      return ss.str();
    }

    [[nodiscard]] std::string toRGBAString() const
    {
      return std::format("rgba({},{},{},{});", static_cast<int>(r), static_cast<int>(g), static_cast<int>(b), a);
    }

    [[nodiscard]] uint32_t pack() const
    {
      const uint32_t packedColor = static_cast<uint32_t>(r) << 24 | static_cast<uint32_t>(g) << 16
          | static_cast<uint32_t>(b) << 8 | static_cast<uint32_t>(a * maxTFixed);
      return packedColor;
    }

    std::tuple<float, float, float, float> normalized() const
    {
      constexpr auto factor = static_cast<float>(maxTFixed);
      return std::make_tuple(r / factor, g / factor, b / factor, a);
    }

    Color multiply(float f) const
    {
      return Color { static_cast<tColorValueType>(r * f), static_cast<tColorValueType>(g * f),
                     static_cast<tColorValueType>(b * f), a };
    }

    static constexpr Color BACKGROUND()
    {
      return { 24, 24, 24, 1 };
    }

    template <IntegralOrChar tFixed> static constexpr Color RGB(tFixed r, tFixed g, tFixed b)
    {
      return Color(r, g, b, 1);
    }

    template <IntegralOrChar tFixed> static constexpr Color RGBA(tFixed r, tFixed g, tFixed b, tFixed a)
    {
      return Color(r, g, b, a);
    }

    static constexpr Color RED()
    {
      return { 255, 0, 0, 1.0 };
    }

    static constexpr Color GREEN()
    {
      return { 0, 255, 0 };
    }

    static constexpr Color BLUE()
    {
      return { 0, 0, 255 };
    }

    static constexpr Color BLACK()
    {
      return { 0, 0, 0 };
    }

    static constexpr Color WHITE()
    {
      return { 255, 255, 255 };
    }

    static constexpr Color GRAY()
    {
      return { 128, 128, 128 };
    }

    Color& TRANSPARENT(float alpha)
    {
      a = alpha;
      return *this;
    }

    static constexpr Color TRANSPARENT(Color c, float alpha)
    {
      return { c.r, c.g, c.b, alpha };
    }

    static constexpr Color TRANSPARENT()
    {
      return { 0, 0, 0, 0 };
    }
  };
}