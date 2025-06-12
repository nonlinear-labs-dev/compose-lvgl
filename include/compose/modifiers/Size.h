#pragma once

namespace Compose
{
  struct Size
  {
    constexpr static int c_screenWidthInPixels = 480;
    constexpr static int c_screenHeightInPixels = 128;

    static Size ScreenSize()
    {
      return Size { c_screenWidthInPixels, c_screenHeightInPixels };
    }

    int w = {};
    int h = {};
  };

  struct FixedSize : Size
  {
  };

  struct MinSize : Size
  {
    static MinSize GreedyWidth()
    {
      return MinSize { c_screenWidthInPixels, 0 };
    }
  };
}