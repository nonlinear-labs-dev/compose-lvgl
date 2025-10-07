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

    explicit Application(Window::Backend backend, Size size = Size::ScreenSizeAudioBox());
    void runBlocking(const tCallback &callback) const;

    void operator<<(const tCallback &callback) const
    {
      runBlocking(callback);
    }

   private:
    Window::Backend m_backend;
    Size m_size;
  };
}

#define APPLICATION(backend, ...) Compose::Application(backend __VA_OPT__(, __VA_ARGS__)) << [=](Compose::Window & it)
