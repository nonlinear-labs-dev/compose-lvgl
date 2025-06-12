#pragma once
#include <reactive/Deferrer.h>

template <typename tDeps> class ComposeApplication
{
 public:
  ComposeApplication(tDeps& deps, const std::string& applicationName)
      : m_deps(deps)
  {
  }

  ~ComposeApplication() = default;

  void run()
  {
  }

  void attach(Gtk::Window& wnd)
  {
  }

  void quit()
  {
    Reactive::Deferrer deferrer;
  }

 private:
  tDeps& m_deps;
};