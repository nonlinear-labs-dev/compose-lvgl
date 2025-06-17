#pragma once
#include "Widget.h"
#include <nltools/system/SingleFileMonitor.h>

namespace Compose
{
  class Container;
  class Fixed;

  class Window : public Widget<Gtk::Window>
  {
   public:
    explicit Window();
    ~Window() override;
    void setFullScreen(bool f) const;

    template <typename T> T &&add(T &&c) const
    {
      setChild(c);
      return std::move(c);
    }

    void setSize(int x, int y) const;

   private:
    void setChild(const auto &widget) const
    {
      const auto window = getHandle();
      window->remove();
      const auto handlePtr = widget.getHandle();
      window->add(*handlePtr);
    }
  };
}

#define LESS_STYLE(path) it.attachLessStyleSheet(path);
