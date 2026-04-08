#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <glibmm/ustring.h>
#include <ft2build.h>
#include <freetype/freetype.h>

namespace Compose
{
  class FreeTypeFont
  {
   public:
    struct TextBounds
    {
      int top {};
      int bottom {};

      [[nodiscard]] int height() const
      {
        return std::max(bottom - top, 0);
      }
    };

    FreeTypeFont(const std::string &fontPath, int height);
    virtual ~FreeTypeFont();

    typedef int32_t tCoordinate;

    enum class Justification
    {
      Left,
      Center,
      Right
    };

    using tSetPixelCB = std::function<void(int, int, unsigned char)>;

    [[nodiscard]] int getStringWidth(const Glib::ustring &str) const;
    [[nodiscard]] int getFontHeight() const;
    [[nodiscard]] int getFontSize() const;
    [[nodiscard]] int getMaxBottomOffset(const Glib::ustring &str) const;
    [[nodiscard]] TextBounds getTextBounds(const Glib::ustring &str) const;
    [[nodiscard]] int getCapHeightPx() const;
    [[nodiscard]] int getAscenderPx() const;

    void draw(const Glib::ustring &text, tCoordinate x, tCoordinate y, const tSetPixelCB &cb) const;
    [[nodiscard]] const std::string &getFontPath() const;

   private:
    FT_Face m_face {};
    int m_fontSize;
    std::string m_fontPath;

    tCoordinate drawLetter(FT_GlyphSlot slot, tCoordinate x, tCoordinate y, const tSetPixelCB &cb) const;
  };
}
