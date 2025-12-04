#include <compose/widgets/Window.h>
#include <lvgl.h>

#include "src/drivers/display/drm/lv_linux_drm.h"
#include "src/drivers/evdev/lv_evdev.h"

namespace Compose
{
  Window::Window(Size size)
  {
    lv_disp_t *disp = lv_linux_drm_create();
    lv_linux_drm_set_file(disp, "/dev/dri/card0", 33);

    lv_indev_t *indev = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event0");
    lv_indev_set_display(indev, disp);

    m_display = disp;
  }
}
