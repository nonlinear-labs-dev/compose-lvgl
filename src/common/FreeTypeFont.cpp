#include "compose/FreeTypeFont.h"
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
      : m_fontSize(height)
      , m_fontPath(fontPath)
  {
    if(!s_library)
      check(FT_Init_FreeType(&s_library), __LINE__);

    check(FT_New_Face(s_library, fontPath.c_str(), 0, &m_face), __LINE__);
    check(FT_Set_Pixel_Sizes(m_face, 0, height), __LINE__, height);
  }

  FreeTypeFont::~FreeTypeFont()
  {
    FT_Done_Face(m_face);
  }

  int FreeTypeFont::getStringWidth(const Glib::ustring &text) const
  {
    uint32_t x = 0;

    for(const auto c : text)
      if(check(FT_Load_Char(m_face, c, FT_LOAD_NO_BITMAP), __LINE__, c))
        x += m_face->glyph->advance.x;

    return static_cast<int>(x >> 6);
  }

  int FreeTypeFont::getFontHeight() const
  {
    return static_cast<int>(m_face->size->metrics.height >> 6);
  }

  int FreeTypeFont::getFontSize() const
  {
    return m_fontSize;
  }

  int FreeTypeFont::getMaxBottomOffset(const Glib::ustring &text) const
  {
    int maxBottom = 0;

    for(const auto c : text)
      if(check(FT_Load_Char(m_face, c, FT_LOAD_RENDER), __LINE__, c))
      {
        const auto glyph = m_face->glyph;
        const auto bottomOffset = m_fontSize - glyph->bitmap_top + static_cast<int>(glyph->bitmap.rows);
        maxBottom = std::max(maxBottom, bottomOffset);
      }

    return maxBottom;
  }

  void FreeTypeFont::draw(const Glib::ustring &text, tCoordinate x, tCoordinate y, const tSetPixelCB &cb) const
  {
    y += m_fontSize;
    x <<= 6;

    for(const auto c : text)
      if(check(FT_Load_Char(m_face, c, FT_LOAD_RENDER), __LINE__, c))
        x += drawLetter(m_face->glyph, x, y, cb);
  }

  inline bool isPixelSet(FT_GlyphSlot slot, int srcX, int srcY)
  {
    const auto index = srcX + srcY * slot->bitmap.width;
    return slot->bitmap.buffer[index];
  }

  inline unsigned char getPixelValue(FT_GlyphSlot slot, int srcX, int srcY)
  {
    const auto index = srcX + srcY * slot->bitmap.width;
    return slot->bitmap.buffer[index];
  }

  FreeTypeFont::tCoordinate FreeTypeFont::drawLetter(FT_GlyphSlot slot, tCoordinate x, tCoordinate y,
                                                     const tSetPixelCB &cb) const
  {
    x >>= 6;

    for(int srcX = 0; srcX < slot->bitmap.width; srcX++)
      for(int srcY = 0; srcY < slot->bitmap.rows; srcY++)
        cb(srcX + x + slot->bitmap_left, (m_fontSize - slot->bitmap_top) + srcY + y - m_fontSize,
           getPixelValue(slot, srcX, srcY));

    return static_cast<tCoordinate>(slot->advance.x);
  }

  const std::string &FreeTypeFont::getFontPath() const
  {
    return m_fontPath;
  }

  int FreeTypeFont::getAscenderPx() const
  {
    return static_cast<int>(m_face->size->metrics.ascender >> 6);
  }

  int FreeTypeFont::getCapHeightPx() const
  {
    // Approximate cap height using glyph 'H' top side bearing
    if(check(FT_Load_Char(m_face, 'H', FT_LOAD_NO_BITMAP), __LINE__, 'H'))
      return static_cast<int>(m_face->glyph->metrics.horiBearingY >> 6);
    return getAscenderPx();
  }
}
