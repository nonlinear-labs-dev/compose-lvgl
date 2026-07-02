#include <compose/widgets/Window.h>

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
}
