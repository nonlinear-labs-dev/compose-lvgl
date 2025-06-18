#include "nltools/logging/Log.h"
#include <compose/widgets/SVGImage.h>

namespace Compose
{
  void SVGImage::setModifier(const SVGPath &p) const
  {
    lv_image_set_src(getHandle(), std::format("S:{}", p.it.c_str()).c_str());
  }

  void SVGImage::setModifier(PrimaryColor c) const
  {
    lv_obj_style_apply_recolor(getHandle(), LV_PART_MAIN,
                               {
                                   .blue = c.b,
                                   .green = c.g,
                                   .red = c.r,
                               });
  }
}
