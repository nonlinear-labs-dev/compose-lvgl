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

    explicit Application(Rect position, Rotation rotation = Rotation::None);

    void runBlocking(const tCallback &callback) const;

    void operator<<(const tCallback &callback) const
    {
      runBlocking(callback);
    }

   private:
    Rect m_position;
    Rotation m_rotation;
  };
}

#define APPLICATION(rect, ...) Compose::Application(rect __VA_OPT__(, ) __VA_ARGS__) << [=](Compose::Window & it)
