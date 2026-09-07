#pragma once
#include <compose/modifiers/Color.h>
#include <compose/modifiers/Rect.h>
#include <lunasvg.h>
#include <optional>

namespace Compose
{
  class DrawContext;

  void renderDocumentToContext(DrawContext &ctx, const lunasvg::Document &doc, Point position, int width, int height,
                               std::optional<Color> colorOverride);
}
