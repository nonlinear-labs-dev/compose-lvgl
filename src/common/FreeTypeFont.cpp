#include "compose/FreeTypeFont.h"
#include <glib.h>

namespace
{
  FT_Library s_library = nullptr;
  constexpr FT_Int32 c_noBitmapLoadFlags = FT_LOAD_NO_BITMAP | FT_LOAD_TARGET_LIGHT;
  constexpr FT_Int32 c_renderLoadFlags = FT_LOAD_RENDER | FT_LOAD_TARGET_LIGHT;

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

  int FreeTypeFont::getStringWidth(const Glib::ustring &text) const
  {
    uint32_t x = 0;

    for(const auto c : text)
    {
      auto face = findFaceForChar(c);
      if(check(FT_Load_Char(face, c, c_noBitmapLoadFlags), __LINE__, c))
        x += face->glyph->advance.x;
    }

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
      auto face = findFaceForChar(c);
      if(check(FT_Load_Char(face, c, c_renderLoadFlags), __LINE__, c))
      {
        const auto glyph = face->glyph;
        const auto bottomOffset = m_fontSize - glyph->bitmap_top + static_cast<int>(glyph->bitmap.rows);
        maxBottom = std::max(maxBottom, bottomOffset);
      }
    }

    return maxBottom;
  }

  void FreeTypeFont::draw(const Glib::ustring &text, tCoordinate x, tCoordinate y, const tSetPixelCB &cb) const
  {
    y += m_fontSize;
    x <<= 6;

    for(const auto c : text)
    {
      auto face = findFaceForChar(c);
      if(check(FT_Load_Char(face, c, c_renderLoadFlags), __LINE__, c))
        x += drawLetter(face->glyph, x, y, cb);
    }
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
    return m_fontPaths.front();
  }

  int FreeTypeFont::getAscenderPx() const
  {
    return static_cast<int>(m_faces.front()->size->metrics.ascender >> 6);
  }

  int FreeTypeFont::getCapHeightPx() const
  {
    // Approximate cap height using glyph 'H' top side bearing
    auto face = findFaceForChar('H');
    if(check(FT_Load_Char(face, 'H', c_noBitmapLoadFlags), __LINE__, 'H'))
      return static_cast<int>(face->glyph->metrics.horiBearingY >> 6);
    return getAscenderPx();
  }

  FT_Face FreeTypeFont::findFaceForChar(char32_t c) const
  {
    for(const auto face : m_faces)
      if(FT_Get_Char_Index(face, c) != 0)
        return face;

    return m_faces.front();
  }
}
