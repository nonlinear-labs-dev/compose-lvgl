#pragma once
#include "compose/FreeTypeFont.h"
#include <functional>
#include <memory>
#include <compose/modifiers/Font.h>

namespace Compose
{
  struct FontStorage
  {
    using tPathBuilder = std::function<std::string(Font)>;

    explicit FontStorage(tPathBuilder builder)
        : buildPath(std::move(builder))
        , fonts()
    {
    }

    FontStorage(const FontStorage &) = delete;
    FontStorage &operator=(const FontStorage &) = delete;

    FreeTypeFont &getFont(const Font &font)
    {
      auto it = fonts.find(font);
      if(it == fonts.end())
      {
        it = fonts.emplace(font, std::make_unique<FreeTypeFont>(buildPath(font), font.size)).first;
      }
      return *it->second;
    }

    tPathBuilder buildPath;
    std::unordered_map<Font, std::unique_ptr<FreeTypeFont>> fonts;
  };
}
