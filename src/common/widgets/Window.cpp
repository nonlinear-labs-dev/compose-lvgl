#include <compose/widgets/Window.h>

#include "src/display/lv_display.h"

namespace Compose
{
  void Window::close()
  {
    if(m_display)
    {
      lv_display_delete(m_display);
      m_display = nullptr;
    }
  }

  Window::~Window()
  {
    if(m_backendCleanup)
    {
      m_backendCleanup();
      m_backendCleanup = nullptr;
    }
  }
}
