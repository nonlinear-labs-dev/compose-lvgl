#include "src/core/lv_obj.h"

#include <compose/widgets/Window.h>
#include "src/misc/lv_types.h"
#include <stdexcept>

namespace Compose
{
  Window::~Window()
  {
    if(m_backendCleanup)
    {
      m_backendCleanup();
      m_backendCleanup = nullptr;
    }
  }

  //void Window::setFullScreen(const bool f) const
  //{
   // [[maybe_unused]] auto v = m_display;
    //throw std::runtime_error("Not implemented");
 // }

  //void Window::setSize(int x, int y) const
  //{
//    lv_display_set_resolution(m_display, x, y);
  //}
}
