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

    explicit Application(Rect position);

    void runBlocking(const tCallback &callback) const;

    void operator<<(const tCallback &callback) const
    {
      runBlocking(callback);
    }

   private:
    Rect m_position;
  };
}

#define APPLICATION(...) Compose::Application(__VA_ARGS__) << [=](Compose::Window & it)
