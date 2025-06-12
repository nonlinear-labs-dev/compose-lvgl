#include <compose/widgets/Window.h>
#include "compose/widgets/Widget.h"
#include "compose/widgets/container/Fixed.h"
#include "compose/widgets/container/Container.h"
#include <nltools/logging/Log.h>
#include <format>

namespace Compose
{
  Window::Window()
      : Widget(new Gtk::Window())
      , m_styleProvider(Gtk::CssProvider::create())
  {
    Gtk::StyleContext::add_provider_for_screen(Gdk::Screen::get_default(), m_styleProvider, 0);
  }

  Window::~Window()
  {
    delete Widget::getHandle();
  }

  void Window::setFullScreen(const bool f) const
  {
    if(f)
      getHandle()->fullscreen();
    else
      getHandle()->unfullscreen();
  }

  void Window::setCSSText(const std::string &css) const
  {
    try
    {
      m_styleProvider->load_from_data(css);
    }
    catch(Gtk::CssProviderError &e)
    {
      nltools::Log::error(e.what());
    }
  }

  void Window::setSize(int x, int y) const
  {
    const auto window = getHandle();
    window->set_resizable(false);
    window->set_default_size(x, y);
    window->set_size_request(x, y);
    window->resize(x, y);
  }

  void Window::attachLessStyleSheet(const std::string &lessPath) const
  {
    m_lessFileMonitor = std::make_unique<nltools::FileTools::SingleFileMonitor>(
        Gio::File::create_for_path(lessPath),
        [&](const nltools::FileTools::SingleFileMonitor::tFile &newFile)
        {
          std::ifstream lessFile(newFile->get_path());
          std::stringstream buffer;
          buffer << lessFile.rdbuf();
          auto ret = compileLess(buffer);
          setCSSText(ret);
        });
  }

  void Window::defineAndUseFont(const std::string &name)
  {
    try
    {
      const std::string css = "* {font-family: '" + name + "', sans-serif;}";
      ensureDataForKeyExistsNonOwning<Gtk::CssProvider>(RoundedCorner::key,
                                                        []
                                                        {
                                                          auto css_provider = Gtk::CssProvider::create();
                                                          Gtk::StyleContext::add_provider_for_screen(
                                                              Gdk::Screen::get_default(), css_provider,
                                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
                                                          return css_provider.get();
                                                        })
          .load_from_data(css);
    }
    catch(const Gtk::CssProviderError &error)
    {
      nltools::Log::error(error.what());
    }
  }
}