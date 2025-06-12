#pragma once
#include "nltools/logging/Log.h"

#include <functional>
#include <optional>
#include <gtkmm/cssprovider.h>
#include <reactive/Computations.h>
#include <nltools-2/NotSyncedException.h>

template <typename tType> class BaseWidget
{
  static constexpr auto c_computationsKey = "Computations";
  constexpr static auto c_connectionsKey = "Connections";

  using tConnection = sigc::connection;
  using tConnections = std::vector<tConnection>;

 public:
  virtual ~BaseWidget() = default;
  using WidgetType = tType;
  using AutorunCB = std::function<void(WidgetType &&)>;

  explicit BaseWidget(WidgetType *w)
      : m_widget(w)
  {
  }

  void addConnection(tConnection &connection) const
  {
    auto &cons = ensureDataForKeyExistsOwning<tConnections>(c_connectionsKey);
    cons.emplace_back(connection);
  }

  template <typename T> T *getData(auto key) const
  {
    if(auto rawPtr = m_widget->get_data(key))
    {
      return static_cast<T *>(rawPtr);
    }
    return nullptr;
  }

  void clearConnections() const
  {
    if(auto *cons = getConnections())
    {
      for(auto c : *cons)
      {
        c.disconnect();
      }
      cons->clear();
    }
  }

  [[nodiscard]] tConnections *getConnections() const
  {
    return getData<tConnections>(c_connectionsKey);
  }

  template <typename T> T &ensureDataForKeyExistsOwning(auto key) const
  {
    auto ret = static_cast<T *>(m_widget->get_data(key));
    if(ret == nullptr)
    {
      ret = new T();
      m_widget->set_data(key, ret, [](auto p) { delete static_cast<T *>(p); });
    }
    return *ret;
  }

  template <typename T> T &ensureDataForKeyExistsNonOwning(auto key, auto fac) const
  {
    auto ret = static_cast<T *>(m_widget->get_data(key));
    if(ret == nullptr)
    {
      ret = fac();
      m_widget->set_data(key, ret, [](auto p) { });
    }
    return *ret;
  }

  [[nodiscard]] virtual WidgetType *getHandle() const
  {
    return m_widget;
  }

  template <typename tCB> void doAutorun(tCB &&cb) const
  {
    getComputations().add(
        [cb = std::forward<tCB>(cb)]
        {
          try
          {
            cb();
          }
          catch(const NotSyncedException &)
          {
          }
          catch(const std::exception &e)
          {
            nltools::Log::warning("Computation::execute() failed:", e.what());
          }
          catch(...)
          {
            nltools::Log::warning("Computation::execute() failed.");
          }
        });
  }

 private:
  [[nodiscard]] Reactive::Computations &getComputations() const
  {
    return ensureDataForKeyExistsOwning<Reactive::Computations>(c_computationsKey);
  }

  WidgetType *m_widget;
};