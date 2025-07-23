#pragma once
#include <memory>
#include <concepts>
#include <functional>

#include "Window.h"

namespace Compose
{
  class Window;

  class Application
  {
   public:
    using tCallback = std::function<void(Window &)>;

    explicit Application(Window::Backend backend);
    void runBlocking(const tCallback &callback) const;

    void operator<<(const tCallback &callback) const
    {
      runBlocking(callback);
    }

   private:
    Window::Backend m_backend;
  };
}

#define APPLICATION(backend) Compose::Application(backend) << [=](Compose::Window & it)
#define FONT(name) it.defineAndUseFont(name);
