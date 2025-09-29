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
    lv_event_dsc_t* resizeHandler = nullptr;
    lv_obj_t* handle = nullptr;

    SVGData(lv_obj_t* handle, CustomDrawingElement::tDrawCB cb);
    ~SVGData();
    void renderToDrawContext(DrawContext& ctx, int width, int height);
  };
}
