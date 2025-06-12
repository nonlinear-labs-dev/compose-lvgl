#include "compose/widgets/Application.h"
#include "compose/widgets/Window.h"

#include <assert.h>
#include <gtkmm/application.h>
#include <reactive/Computations.h>

namespace Compose
{
  Application::Application(const std::string& name)
  {
    assert(g_application_id_is_valid(name.c_str()));
    // m_app = Gtk::Application::create(name);
  }

  void Application::runBlocking(const tCallback& callback) const
  {
    Window window;
    addWindowWhenReady(window);
    Reactive::Computations c;
    c.add([&] { callback(window); });
    // m_app->run();
  }

  void Application::addWindow(Window::WidgetType* windowHandle) const
  {
    // m_app->add_window(*windowHandle);
    windowHandle->show();
  }

  void Application::addWindowWhenReady(const Window& window) const
  {
    // m_app->signal_startup().connect([&] { addWindow(window.getHandle()); });
  }
}