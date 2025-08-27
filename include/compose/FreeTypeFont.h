#pragma once

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

    uint32_t getStringWidth(const Glib::ustring &str) const;
    uint32_t draw(const Glib::ustring &text, tCoordinate x, tCoordinate y, const tSetPixelCB &cb) const;
    int getHeight() const;
    const std::string &getFontPath() const;

   private:
    FT_Face m_face {};
    int m_height;
    std::string m_fontPath;

    tCoordinate drawLetter(FT_GlyphSlot slot, tCoordinate x, tCoordinate y, const tSetPixelCB &cb) const;
  };
}
