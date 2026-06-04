#pragma once
#include <compose/modifiers/SVGPath.h>
#include <lunasvg/lunasvg.h>

namespace Compose
{
  class SVGDocumentCache
  {
   public:
    static const lunasvg::Document *documentFor(const SVGPath &path);
    static const lunasvg::Document *documentFor(const SVGFileContent &content);
  };
}
