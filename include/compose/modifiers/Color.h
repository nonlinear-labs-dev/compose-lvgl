#pragma once
#include <algorithm>
#include <cmath>
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

    constexpr Color() = default;
    constexpr Color(Color&& c) = default;
    constexpr Color(const Color& c) = default;
    constexpr Color& operator=(const Color&) = default;
    constexpr bool operator==(const Color&) const = default;

    [[nodiscard]] std::string toHEXString() const
    {
      std::stringstream ss;
      ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(r) << std::setw(2)
         << static_cast<int>(g) << std::setw(2) << static_cast<int>(b) << std::setw(2)
         << static_cast<int>(a * maxTFixed);
      return ss.str();
    }

    constexpr static Color fromHEXString(const std::string& hexString)
    {
      const auto hashtagOffset = hexString[0] == '#' ? 1 : 0;
      const auto hexPart = hexString.substr(hashtagOffset);

      if(hexPart.size() != 6 && hexPart.size() != 8)
        throw std::invalid_argument("Invalid hex string length");

      const auto parseHex = [](const std::string& hex) -> unsigned int
      { return static_cast<unsigned int>(std::stoul(hex, nullptr, 16)); };

      const auto r = parseHex(hexPart.substr(0, 2));
      const auto g = parseHex(hexPart.substr(2, 2));
      const auto b = parseHex(hexPart.substr(4, 2));
      const auto a = hexPart.size() == 8 ? parseHex(hexPart.substr(6, 2)) : 255;

      return { r, g, b, static_cast<float>(a) / maxTFixed };
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

    [[nodiscard]] uint32_t pack_without_alpha() const
    {
      return (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
    }

    [[nodiscard]] std::tuple<float, float, float, float> normalized() const
    {
      constexpr auto factor = static_cast<float>(maxTFixed);
      return std::make_tuple(r / factor, g / factor, b / factor, a);
    }

    [[nodiscard]] Color multiply(float f) const
    {
      return Color { static_cast<tColorValueType>(r * f), static_cast<tColorValueType>(g * f),
                     static_cast<tColorValueType>(b * f), a };
    }

    static constexpr Color BACKGROUND()
    {
      return { 0, 0, 0, 1 };
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

    static constexpr Color YELLOW()
    {
      return { 255, 255, 0 };
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

    static constexpr Color GRAY(auto g)
    {
      return { g, g, g };
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

    struct HSV
    {
      float h = 0.0f;
      float s = 0.0f;
      float v = 0.0f;
    };

    [[nodiscard]] HSV toHSV() const
    {
      const float rf = static_cast<float>(r) / maxTFixed;
      const float gf = static_cast<float>(g) / maxTFixed;
      const float bf = static_cast<float>(b) / maxTFixed;

      const float cmax = std::fmax(rf, std::fmax(gf, bf));
      const float cmin = std::fmin(rf, std::fmin(gf, bf));
      const float delta = cmax - cmin;

      HSV hsv;
      hsv.v = cmax;
      hsv.s = (hsv.v <= 1e-6f) ? 0.0f : (delta / hsv.v);

      if(delta > 1e-6f)
      {
        if(cmax == rf)
          hsv.h = 60.0f * std::fmod(((gf - bf) / delta), 6.0f);
        else if(cmax == gf)
          hsv.h = 60.0f * (((bf - rf) / delta) + 2.0f);
        else
          hsv.h = 60.0f * (((rf - gf) / delta) + 4.0f);

        if(hsv.h < 0.0f)
          hsv.h += 360.0f;
      }

      return hsv;
    }

    [[nodiscard]] static Color fromHSV(float h, float s, float v, float alpha = 1.0f)
    {
      s = std::clamp(s, 0.0f, 1.0f);
      v = std::clamp(v, 0.0f, 1.0f);

      h = std::fmod(h, 360.0f);
      if(h < 0.0f)
        h += 360.0f;

      const float c = v * s;
      const float hh = h / 60.0f;
      const float x = c * (1.0f - std::fabs(std::fmod(hh, 2.0f) - 1.0f));
      const float m = v - c;

      float rp = 0.0f;
      float gp = 0.0f;
      float bp = 0.0f;

      if(s <= 1e-6f)
      {
        rp = v;
        gp = v;
        bp = v;
      }
      else if(hh < 1.0f)
      {
        rp = c;
        gp = x;
      }
      else if(hh < 2.0f)
      {
        rp = x;
        gp = c;
      }
      else if(hh < 3.0f)
      {
        gp = c;
        bp = x;
      }
      else if(hh < 4.0f)
      {
        gp = x;
        bp = c;
      }
      else if(hh < 5.0f)
      {
        rp = x;
        bp = c;
      }
      else
      {
        rp = c;
        bp = x;
      }

      auto toByte = [](float value) -> tColorValueType
      {
        return static_cast<tColorValueType>(std::round(std::clamp(value, 0.0f, 1.0f) * static_cast<float>(maxTFixed)));
      };

      return Color { toByte(rp + m), toByte(gp + m), toByte(bp + m), alpha };
    }

    [[nodiscard]] Color withHsvScaled(float hFactor, float sFactor, float vFactor) const
    {
      auto hsv = toHSV();
      hsv.h = std::fmod(hsv.h * hFactor, 360.0f);
      if(hsv.h < 0.0f)
        hsv.h += 360.0f;
      hsv.s = std::clamp(hsv.s * sFactor, 0.0f, 1.0f);
      hsv.v = std::clamp(hsv.v * vFactor, 0.0f, 1.0f);
      return fromHSV(hsv.h, hsv.s, hsv.v, a);
    }

    [[nodiscard]] Color dimBrightness(float factor) const
    {
      factor = std::clamp(factor, 0.0f, 1.0f);
      return withHsvScaled(1.0f, 1.0f, factor);
    }
  };
}
