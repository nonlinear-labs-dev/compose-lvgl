#include "compose/FreeTypeFont.h"
#include <glib.h>

namespace
{
  static FT_Library s_library = nullptr;

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
      : m_height(height)
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

  uint32_t FreeTypeFont::getStringWidth(const Glib::ustring &text) const
  {
    uint32_t x = 0;

    for(auto c : text)
      if(check(FT_Load_Char(m_face, c, FT_LOAD_NO_BITMAP), __LINE__, c))
        x += m_face->glyph->advance.x;

    return x >> 6;
  }

  uint32_t FreeTypeFont::draw(const Glib::ustring &text, tCoordinate x, tCoordinate y, const tSetPixelCB &cb)
  {
    y += m_height;
    auto oldX = x;
    x <<= 6;

    for(auto c : text)
      if(check(FT_Load_Char(m_face, c, FT_LOAD_RENDER), __LINE__, c))
        x += drawLetter(m_face->glyph, x, y, cb);

    x >>= 6;
    return x - oldX;
  }

  inline bool isPixelSet(FT_GlyphSlot slot, int srcX, int srcY)
  {
    auto index = srcX + srcY * slot->bitmap.width;
    return slot->bitmap.buffer[index];
  }

  inline unsigned char getPixelValue(FT_GlyphSlot slot, int srcX, int srcY)
  {
    auto index = srcX + srcY * slot->bitmap.width;
    return slot->bitmap.buffer[index];
  }

  FreeTypeFont::tCoordinate FreeTypeFont::drawLetter(FT_GlyphSlot slot, FreeTypeFont::tCoordinate x,
                                                     FreeTypeFont::tCoordinate y, const tSetPixelCB &cb)
  {
    x >>= 6;

    for(int srcX = 0; srcX < slot->bitmap.width; srcX++)
      for(int srcY = 0; srcY < slot->bitmap.rows; srcY++)
      // if(isPixelSet(slot, srcX, srcY))
      {
        cb(srcX + x + slot->bitmap_left, (m_height - slot->bitmap_top) + srcY + y - m_height,
           getPixelValue(slot, srcX, srcY));
      }

    return slot->advance.x;
  }

  int FreeTypeFont::getHeight() const
  {
    return m_height;
  }

  const std::string &FreeTypeFont::getFontPath() const
  {
    return m_fontPath;
  }
}
