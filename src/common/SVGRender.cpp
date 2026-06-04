#include <compose/SVGRender.h>
#include <compose/widgets/DrawContext.h>

namespace Compose
{
  void renderDocumentToContext(DrawContext &ctx, const lunasvg::Document &doc, Point position, int width, int height,
                               std::optional<Color> colorOverride)
  {
    const auto bitmap = doc.renderToBitmap(width, height);
    if(bitmap.isNull())
      return;

    ctx.putBitmap({ bitmap.width(), bitmap.height(), bitmap.stride(), bitmap.data() }, position, colorOverride);
  }
}
