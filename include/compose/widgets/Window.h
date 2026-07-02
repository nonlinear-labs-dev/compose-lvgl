#pragma once
#include "compose/modifiers/Rect.h"
#include "compose/modifiers/Rotation.h"
#include "container/Container.h"
#include "src/misc/lv_types.h"
#include <functional>
#include <vector>

namespace Compose
{
  class Widget;
  class Window
  {
   public:
    explicit Window(Rect position, Rotation rotation = Rotation::None);
    ~Window();

    template <typename T> static T&& add(T&& it)
    {
      lv_screen_load(it.getHandle());
      return std::forward<T>(it);
    }

    [[nodiscard]] lv_display_t* getDisplay() const
    {
      return m_display;
    }

   private:
    lv_display_t* m_display { nullptr };
    lv_indev_t* m_mouse { nullptr };
    lv_indev_t* m_mouseWheel { nullptr };
    lv_indev_t* m_keyboard { nullptr };
    std::vector<lv_indev_t*> m_touchIndevs;
    std::function<void()> m_backendCleanup;
  };
}
