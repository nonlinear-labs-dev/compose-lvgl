#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <glibmm/ustring.h>
#include <ft2build.h>
#include <freetype/freetype.h>

namespace Compose
{
  class FreeTypeFont
  {
   public:
    FreeTypeFont(const std::string &fontPath, int height);
    FreeTypeFont(std::vector<std::string> fontPaths, int height);
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
    [[nodiscard]] int getCapHeightPx() const;
    [[nodiscard]] int getAscenderPx() const;

    void draw(const Glib::ustring &text, tCoordinate x, tCoordinate y, const tSetPixelCB &cb) const;
    [[nodiscard]] const std::string &getFontPath() const;

   private:
    struct CachedGlyph
    {
      CachedGlyph(FT_GlyphSlot slot);
      FT_Fixed advance_x {};
      int bitmap_left {};
      int bitmap_top {};
      unsigned int width {};
      unsigned int rows {};
      FT_Pos hori_bearing_y {};
      std::vector<uint8_t> pixels {};
    };

    std::vector<FT_Face> m_faces;
    int m_fontSize;
    std::vector<std::string> m_fontPaths;
    mutable std::unordered_map<std::uint32_t, CachedGlyph> m_glyphCache;

    const CachedGlyph &glyphFor(std::uint32_t codepoint) const;
    tCoordinate drawLetterFromCache(const CachedGlyph &g, tCoordinate x, tCoordinate y, const tSetPixelCB &cb) const;
    [[nodiscard]] FT_Face findFaceForChar(char32_t c) const;
  };
}
