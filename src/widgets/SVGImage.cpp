#include "nltools/logging/Log.h"
#include <compose/widgets/SVGImage.h>

namespace Compose
{
  void SVGImage::setModifier(const SVGPath &p) const
  {
    lv_image_set_src(getHandle(), std::format("S:{}", p.it.c_str()).c_str());

    const auto viewboxSize = getViewboxSize();
    if(p.size.has_value())
    {
      const auto [w, h] = p.size.value();
      lv_obj_set_size(getHandle(), p.size.value().w, p.size.value().h);
      constexpr static auto baseZoom = 256;
      const auto scaleX = static_cast<float>(w) / static_cast<float>(viewboxSize.w);
      const auto scaleY = static_cast<float>(h) / static_cast<float>(viewboxSize.h);
      lv_image_set_scale_x(getHandle(), scaleX * baseZoom);
      lv_image_set_scale_y(getHandle(), scaleY * baseZoom);
    }
  }

  void SVGImage::setModifier(PrimaryColor c) const
  {
    lv_obj_set_style_image_recolor_opa(getHandle(), LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_image_recolor(getHandle(),
                                   {
                                       .blue = c.b,
                                       .green = c.g,
                                       .red = c.r,
                                   },
                                   LV_PART_MAIN);
  }

  Size SVGImage::getViewboxSize() const
  {
    return Size { lv_image_get_src_width(getHandle()), lv_image_get_src_height(getHandle()) };
  }
}
