#include "compose/state/SVGData.h"
#include <compose/SVGRender.h>

namespace Compose
{
  SVGData::SVGData(CustomDrawingElement::tDrawCB cb)
      : drawCallback(std::move(cb))

  {
  }

  void SVGData::renderToDrawContext(DrawContext &ctx, int width, int height) const
  {
    const auto doc = document.get().get();
    if(!doc)
      return;

    renderDocumentToContext(ctx, *doc, { 0, 0 }, width, height, color.get());
  }
}
