#include "compose/widgets/DrawContext.h"
#include <compose/widgets/CustomDrawingElement.h>

namespace Compose
{
  CustomDrawingElement::CustomDrawingElement(WidgetType *handle)
      : Widget(handle)
  {
  }

  void CustomDrawingElement::setDrawCall(tDrawCB &&draw) const
  {
    auto connection = getHandle()->signal_draw().connect(
        [handle = getHandle(), draw,
         computations = std::make_shared<Reactive::Computations>()](const Cairo::RefPtr<Cairo::Context> &ctx) mutable
        {
          computations = std::make_shared<Reactive::Computations>();

          computations->add(
              [handle, draw, ctx, init = true]() mutable
              {
                if(!std::exchange(init, false))
                {
                  handle->queue_draw();
                  return;
                }

                const auto allocation = handle->get_allocation();
                const int width = allocation.get_width();
                const int height = allocation.get_height();

                CairoDrawContext ourDrawContext { ctx };
                draw(ourDrawContext, width, height);
              });

          return true;
        });
    addConnection(connection);
  }
}