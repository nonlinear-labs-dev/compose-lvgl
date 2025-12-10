#include <compose/widgets/Window.h>
#include <lvgl.h>
#include "src/drivers/display/fb/lv_linux_fbdev.h"
#include "src/drivers/evdev/lv_evdev.h"

namespace Compose
{
  Window::Window(Rect position)
  {
    lv_disp_t *disp = lv_linux_fbdev_create();
    lv_linux_fbdev_set_file(disp, "/dev/fb0");

    lv_indev_t *indev = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event0");
    lv_indev_set_display(indev, disp);

    lv_display_set_offset(disp, position.pos.x, position.pos.y);
    lv_display_set_resolution(disp, position.size.w, position.size.h);

    m_display = disp;
  }
}