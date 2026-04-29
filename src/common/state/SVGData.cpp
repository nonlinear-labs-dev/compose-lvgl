#include "compose/state/SVGData.h"
#include "compose/widgets/DrawContext.h"
#include "compose/modifiers/Color.h"
#include "compose/modifiers/Rect.h"
#include "reactive/Computation.h"

namespace Compose
{
  SVGData::SVGData(CustomDrawingElement::tDrawCB cb)
      : drawCallback(std::move(cb))

  {
  }

  void SVGData::renderToDrawContext(DrawContext& ctx, int width, int height) const
  {
    const auto doc = document.get().get();
    if(!doc)
      return;

    const auto bitmap = doc->renderToBitmap(width, height);

    if(!bitmap.valid())
      return;

    const auto* data = bitmap.data();
    const auto stride = bitmap.stride();

    ctx.putBitmap({ width, height, stride, data }, { 0, 0 }, color.get());
  }
}
