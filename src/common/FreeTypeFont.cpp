#include "compose/FreeTypeFont.h"
#include <algorithm>
#include <glib.h>

namespace
{
  FT_Library s_library = nullptr;

  template <typename... Args> bool check(FT_Error err, Args... args)
  {
    if(err)
      g_error("FreeType error: %s", FT_Error_String(err));

    return err == 0;
  }
}

namespace Compose
{
  FreeTypeFont::FreeTypeFont(const std::string &fontPath, int height)
      : FreeTypeFont(std::vector<std::string> { fontPath }, height)
  {
  }

  FreeTypeFont::FreeTypeFont(std::vector<std::string> fontPaths, int height)
      : m_faces()
      , m_fontSize(height)
      , m_fontPaths(std::move(fontPaths))
  {
    if(!s_library)
      check(FT_Init_FreeType(&s_library), __LINE__);

    for(const auto &fontPath : m_fontPaths)
    {
      FT_Face face = nullptr;
      check(FT_New_Face(s_library, fontPath.c_str(), 0, &face), __LINE__);
      check(FT_Set_Pixel_Sizes(face, 0, height), __LINE__, height);
      m_faces.push_back(face);
    }
  }

  FreeTypeFont::~FreeTypeFont()
  {
    for(const auto face : m_faces)
      FT_Done_Face(face);
  }

  FreeTypeFont::CachedGlyph::CachedGlyph(const FT_GlyphSlot slot)
  {
    advance_x = slot->advance.x;
    bitmap_left = slot->bitmap_left;
    bitmap_top = slot->bitmap_top;
    width = slot->bitmap.width;
    rows = slot->bitmap.rows;
    hori_bearing_y = slot->metrics.horiBearingY;
    if(slot->bitmap.buffer != nullptr && width > 0 && rows > 0)
      pixels.assign(slot->bitmap.buffer, slot->bitmap.buffer + width * rows);
  }

  const FreeTypeFont::CachedGlyph &FreeTypeFont::glyphFor(const std::uint32_t codepoint) const
  {
    if(const auto found = m_glyphCache.find(codepoint); found != m_glyphCache.end())
      return found->second;
    const auto face = findFaceForChar(codepoint);
    check(FT_Load_Char(face, codepoint, FT_LOAD_RENDER), __LINE__, codepoint);
    return m_glyphCache.emplace(codepoint, std::move(CachedGlyph(face->glyph))).first->second;
  }

  int FreeTypeFont::getStringWidth(const Glib::ustring &text) const
  {
    std::uint32_t x = 0;

    for(const auto c : text)
      x += glyphFor(c).advance_x;

    return static_cast<int>(x >> 6);
  }

  int FreeTypeFont::getFontHeight() const
  {
    return static_cast<int>(m_faces.front()->size->metrics.height >> 6);
  }

  int FreeTypeFont::getFontSize() const
  {
    return m_fontSize;
  }

  int FreeTypeFont::getMaxBottomOffset(const Glib::ustring &text) const
  {
    int maxBottom = 0;

    for(const auto c : text)
    {
      const auto &g = glyphFor(c);
      const auto bottomOffset = m_fontSize - g.bitmap_top + static_cast<int>(g.rows);
      maxBottom = std::max(maxBottom, bottomOffset);
    }

    return maxBottom;
  }

  void FreeTypeFont::draw(const Glib::ustring &text, tCoordinate x, tCoordinate y, const tSetPixelCB &cb) const
  {
    y += m_fontSize;
    x <<= 6;

    for(const auto c : text)
      x += drawLetterFromCache(glyphFor(c), x, y, cb);
  }

  FreeTypeFont::tCoordinate FreeTypeFont::drawLetterFromCache(const CachedGlyph &g, tCoordinate x, tCoordinate y,
                                                              const tSetPixelCB &cb) const
  {
    x >>= 6;

    for(unsigned int glyphY = 0; glyphY < g.rows; glyphY++)
    {
      const unsigned int rowOff = glyphY * g.width;
      for(unsigned int glyphX = 0; glyphX < g.width; glyphX++)
      {
        const auto globalX = static_cast<int>(glyphX) + x + g.bitmap_left;
        const auto globalY = (m_fontSize - g.bitmap_top) + static_cast<int>(glyphY) + y - m_fontSize;
        cb(globalX, globalY, g.pixels[rowOff + glyphX]);
      }
    }

    return static_cast<tCoordinate>(g.advance_x);
  }

  const std::string &FreeTypeFont::getFontPath() const
  {
    return m_fontPaths.front();
  }

  int FreeTypeFont::getAscenderPx() const
  {
    return static_cast<int>(m_faces.front()->size->metrics.ascender >> 6);
  }

  int FreeTypeFont::getCapHeightPx() const
  {
    return static_cast<int>(glyphFor(static_cast<std::uint32_t>('H')).hori_bearing_y >> 6);
  }

  FT_Face FreeTypeFont::findFaceForChar(char32_t c) const
  {
    for(const auto face : m_faces)
      if(FT_Get_Char_Index(face, c) != 0)
        return face;

    return m_faces.front();
  }
}
