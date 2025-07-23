#pragma once
#include "container/Container.h"
#include "src/misc/lv_types.h"

namespace Compose
{
  class Widget;
  class Window
  {
   public:
    enum class Backend
    {
      SDL,
      Framebuffer
    };

    explicit Window(Backend end, Size size);
    ~Window();
    void setFullScreen(bool f) const;
    void setSize(int x, int y) const;

    template <typename T> static T&& add(T&& it)
    {
      lv_screen_load(it.getHandle());
      return std::forward<T>(it);
    }

   private:
    lv_display_t* m_display { nullptr };
    lv_indev_t* m_mouse { nullptr };
    lv_indev_t* m_mouseWheel { nullptr };
    lv_indev_t* m_keyboard { nullptr };
  };
}
