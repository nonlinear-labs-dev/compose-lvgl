#pragma once
#include "compose/FreeTypeFont.h"
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include <compose/modifiers/Font.h>

namespace Compose
{
  struct FontStorage
  {
    using tSinglePathBuilder = std::function<std::string(const Font &)>;
    using tPathBuilder = std::function<std::vector<std::string>(const Font &)>;

    explicit FontStorage(tSinglePathBuilder builder)
        : FontStorage([builder = std::move(builder)](const Font &font) {
            std::vector<std::string> paths;
            paths.reserve(1 + font.fallbackBaseNames.size());
            paths.push_back(builder(font));

            for(const auto &fallbackName : font.fallbackBaseNames)
            {
              auto fallbackFont = font;
              fallbackFont.baseName = fallbackName;
              fallbackFont.fallbackBaseNames.clear();
              paths.push_back(builder(fallbackFont));
            }

            return paths;
          })
    {
    }

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
