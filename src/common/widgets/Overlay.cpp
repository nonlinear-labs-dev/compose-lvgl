#include "compose/widgets/Overlay.h"

namespace Compose
{
  Widget::WidgetType *Overlay::makeContent(Widget &creator)
  {
    auto anchor = lv_obj_create(creator.getHandle());
    lv_obj_add_flag(anchor, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(anchor, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_set_size(anchor, 0, 0);

    auto backdrop = lv_obj_create(lv_layer_top());
    lv_obj_add_flag(backdrop, LV_OBJ_FLAG_FLOATING);
    lv_obj_add_flag(backdrop, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_set_size(backdrop, lv_display_get_horizontal_resolution(nullptr), lv_display_get_vertical_resolution(nullptr));
    lv_obj_set_pos(backdrop, 0, 0);
    lv_obj_set_style_border_width(backdrop, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(backdrop, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(backdrop, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(backdrop, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(backdrop, LV_OPA_70, LV_PART_MAIN);
    lv_obj_set_layout(backdrop, LV_LAYOUT_NONE);
    lv_obj_add_flag(backdrop, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(backdrop, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_event_cb(
        anchor, [](lv_event_t *e) { lv_obj_delete(static_cast<lv_obj_t *>(lv_event_get_user_data(e))); }, LV_EVENT_DELETE, backdrop);

    Widget backdropWidget(backdrop);
    backdropWidget.clicked << [backdrop](Position p) {
      if(auto cb = Widget(backdrop).getData<DismissedCB>(c_dismissedKey))
        return (*cb)(p);
      return false;
    };

    // Pass as BaseWidget& explicitly - a plain Widget argument would pick the
    // copy constructor and alias the backdrop instead of creating a child.
    Widget content(static_cast<BaseWidget &>(backdropWidget));
    lv_obj_remove_flag(content.getHandle(), LV_OBJ_FLAG_EVENT_BUBBLE);
    return content.getHandle();
  }
}
