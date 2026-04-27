#pragma once
#include <compose/modifiers/Modifiers.h>
#include <memory>
#include <reactive/Var.h>
#include <compose/widgets/CustomDrawingElement.h>
#include <lunasvg/lunasvg.h>

namespace Compose
{
  struct SVGData
  {
    using tDocumentPtr = std::unique_ptr<lunasvg::Document>;

    Reactive::Var<tDocumentPtr> document { nullptr };
    CustomDrawingElement::tDrawCB drawCallback;
    Reactive::Var<std::optional<PrimaryColor>> color;

    SVGData(CustomDrawingElement::tDrawCB cb);
    ~SVGData() = default;
    void renderToDrawContext(DrawContext& ctx, int width, int height) const;
  };
}
