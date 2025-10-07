#pragma once

namespace Compose
{
  struct Size
  {
    constexpr static int c_screenAudioBoxWidthInPixels = 480;
    constexpr static int c_screenAudioBoxHeightInPixels = 128;

    constexpr static int c_screenC16WidthInPixels = 1025;
    constexpr static int c_screenC16HeightInPixels = 600;

    constexpr static Size ScreenSizeAudioBox()
    {
      return { c_screenAudioBoxWidthInPixels, c_screenAudioBoxHeightInPixels };
    }

    constexpr static Size ScreenSizeC16()
    {
      return { c_screenC16WidthInPixels, c_screenC16HeightInPixels };
    }

    template <typename T> constexpr Size operator*(T v) const
    {
      return Size { w * v, h * v };
    }

    int w = {};
    int h = {};

    bool operator==(const Size &) const = default;
  };

  struct FixedSize : Size
  {
    constexpr static FixedSize ScreenSizeAudioBox()
    {
      return { Size::ScreenSizeAudioBox() };
    }

    constexpr static FixedSize ScreenSizeC16()
    {
      return { Size::ScreenSizeC16() };
    }

    bool operator==(const FixedSize &) const = default;
  };
}