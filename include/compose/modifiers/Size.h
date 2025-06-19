#pragma once

namespace Compose
{
  struct Size
  {
    constexpr static int c_screenWidthInPixels = 480;
    constexpr static int c_screenHeightInPixels = 128;
    constexpr static Size ScreenSize()
    {
      return Size { c_screenWidthInPixels, c_screenHeightInPixels };
    }

    template <typename T> constexpr Size operator*(T v) const
    {
      return Size { w * v, h * v };
    }

    int w = {};
    int h = {};
  };

  struct FixedSize : Size
  {
  };

  struct [[deprecated]]  MinSize : Size
  {
    static MinSize GreedyWidth()
    {
      return MinSize { c_screenWidthInPixels, 0 };
    }
  };
}