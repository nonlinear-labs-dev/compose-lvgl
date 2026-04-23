#include "compose/state/CanvasData.h"
#include "compose/widgets/DrawContext.h"
#include <compose/widgets/CustomDrawingElement.h>

#include "reactive/Computation.h"
#include "src/widgets/canvas/lv_canvas.h"
#include "src/draw/lv_draw_buf.h"
#include "src/misc/lv_color.h"

namespace Compose
{
  void CustomDrawingElement::setDrawCall(tDrawCB &&draw) const
  {
    assert(!doesDataForKeyExist<
           CanvasData>());  // CanvasData should not exist, setting a new render callback is prohibited

    Widget(getHandle())
        .doAutorun(
            [draw = std::move(draw), handle = getHandle()]
            {
              const Widget widget(handle);

              auto &canvasData = widget.ensureDataForKeyExistsOwning<CanvasData>(c_canvasData, [handle, d = draw]
                                                                                 { return new CanvasData(handle, d); });
              (void) canvasData.m_buffer.get();

              const auto w = lv_obj_get_width(handle);
              const auto h = lv_obj_get_height(handle);

              LVGLDrawContext drawContext(*handle);
              try
              {
                canvasData.m_drawCallback(drawContext, w, h);
              }
              catch(std::exception &)
              {
              }
            });
  }
}