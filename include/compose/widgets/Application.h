#pragma once
#include <memory>
#include <concepts>
#include <functional>
#include <gtkmm.h>

#include "Window.h"

namespace Compose
{
  class Window;

  class Application
  {
   public:
    using tCallback = std::function<void(Window &)>;

    explicit Application(const std::string &name);
    void runBlocking(const tCallback &callback) const;

    void operator<<(const tCallback &callback)
    {
      runBlocking(callback);
    }

   private:
    // Glib::RefPtr<Gtk::Application> m_app;

    void addWindow(Window::WidgetType *windowHandle) const;
    void addWindowWhenReady(const Window &window) const;
  };
}

#define APPLICATION(name) Compose::Application(name) << [=](Compose::Window & it)
#define FONT(name) it.defineAndUseFont(name);
