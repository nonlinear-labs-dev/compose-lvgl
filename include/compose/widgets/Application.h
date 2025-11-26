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

    explicit Application();
    void runBlocking(const tCallback &callback) const;

    void operator<<(const tCallback &callback) const
    {
      runBlocking(callback);
    }
  };
}

#define APPLICATION() Compose::Application() << [=](Compose::Window & it)
