#include "nltools/logging/Log.h"
#include <compose/widgets/SVGImage.h>

namespace Compose
{
  void SVGImage::setModifier(const SVGPath &p) const
  {
    persistModifier(p);
    lv_image_set_src(getHandle(), std::format("S:{}", p.it.c_str()).c_str());

    if(p.size.has_value())
    {
      const auto viewboxSize = getViewboxSize();
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
    persistModifier(c);
    lv_obj_set_style_image_recolor_opa(getHandle(), static_cast<float>(LV_OPA_COVER) * c.a, LV_PART_MAIN);
    lv_obj_set_style_image_recolor(getHandle(),
                                   {
                                       .blue = c.b,
                                       .green = c.g,
                                       .red = c.r,
                                   },
                                   LV_PART_MAIN);

#ifdef ENABLE_DEBUG_SHADOWS
    lv_obj_set_style_bg_image_recolor_opa(getHandle(), static_cast<float>(LV_OPA_COVER) * c.a, LV_PART_MAIN);
    lv_obj_set_style_bg_image_recolor(getHandle(),
                                      {
                                          .blue = c.b,
                                          .green = c.g,
                                          .red = c.r,
                                      },
                                      LV_PART_MAIN);

    lv_obj_set_style_shadow_spread(getHandle(), 10, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(getHandle(), static_cast<float>(LV_OPA_COVER) * c.a, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(getHandle(),
                                  {
                                      .blue = c.b,
                                      .green = c.g,
                                      .red = c.r,
                                  },
                                  LV_PART_MAIN);
    lv_obj_set_style_shadow_width(getHandle(), 10, LV_PART_MAIN);
#endif
  }

  Size SVGImage::getViewboxSize() const
  {
    return Size { lv_image_get_src_width(getHandle()), lv_image_get_src_height(getHandle()) };
  }
}
