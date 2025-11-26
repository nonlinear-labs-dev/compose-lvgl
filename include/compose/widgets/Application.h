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

    explicit Application(Size size);

    void runBlocking(const tCallback &callback) const;

    void operator<<(const tCallback &callback) const
    {
      runBlocking(callback);
    }

   private:
    Size m_size;
  };
}

#define APPLICATION(...) Compose::Application(backend __VA_OPT__(, __VA_ARGS__)) << [=](Compose::Window & it)
