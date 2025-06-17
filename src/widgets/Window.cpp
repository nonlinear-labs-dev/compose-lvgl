#include <compose/widgets/Window.h>
#include "compose/widgets/Widget.h"
#include "compose/widgets/container/Fixed.h"
#include "compose/widgets/container/Container.h"
#include <nltools/logging/Log.h>
#include <format>

namespace Compose
{
  Window::Window()
      : Widget(new Gtk::Window())
  {
  }

  Window::~Window()
  {
    delete Widget::getHandle();
  }

  void Window::setFullScreen(const bool f) const
  {
    if(f)
      getHandle()->fullscreen();
    else
      getHandle()->unfullscreen();
  }

  void Window::setSize(int x, int y) const
  {
    const auto window = getHandle();
    window->set_resizable(false);
    window->set_default_size(x, y);
    window->set_size_request(x, y);
    window->resize(x, y);
  }
}