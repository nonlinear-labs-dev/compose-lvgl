#pragma once
#include "src/core/lv_obj.h"
#include "src/misc/lv_types.h"

#include <functional>
#include <optional>
#include <format>
#include <iostream>
#include <unordered_map>
#include <string>
#include <memory>
#include <utility>
#include <reactive/Computations.h>
#include <nltools-2/NotSyncedException.h>

class BaseWidget
{
 public:
  using tType = lv_obj_t;
  static constexpr auto c_computationsKey = "Computations";
  static constexpr auto c_nameKey = "Name";
  static constexpr auto c_leftClickKey = "LeftClick";
  static constexpr auto c_longClickKey = "LongClick";
  static constexpr auto c_stateChangeKey = "StateChange";
  static constexpr auto c_canvasData = "CanvasData";

  struct UserDataEntry
  {
    void* data;
    std::function<void(void*)> cleanup;

    UserDataEntry(void* d, std::function<void(void*)> c)
        : data(d)
        , cleanup(std::move(c))
    {
    }

    ~UserDataEntry()
    {
      if(cleanup && data)
      {
        cleanup(data);
      }
    }
  };

  struct UserDataStorage
  {
    std::unordered_map<std::string, std::unique_ptr<UserDataEntry>> entries;

    ~UserDataStorage()
    {
      entries.clear();
    }
  };

  virtual ~BaseWidget() = default;
  using WidgetType = tType;
  using AutorunCB = std::function<void(WidgetType&&)>;

  explicit BaseWidget(WidgetType* w)
      : m_widget(w)
  {
  }

  void setID(const std::string& id) const
  {
    ensureDataForKeyExistsOwning<std::string>(c_nameKey) = id;
  }

  [[nodiscard]] const std::string& getID() const
  {
    return ensureDataForKeyExistsOwning<std::string>(c_nameKey);
  }

  template <typename T> T* getData(auto key) const
  {
    auto storage = getUserDataStorage();
    if(!storage)
      return nullptr;

    auto it = storage->entries.find(key);
    if(it != storage->entries.end() && it->second)
    {
      return static_cast<T*>(it->second->data);
    }
    return nullptr;
  }

  template <typename T> T& ensureDataForKeyExistsOwning(auto key) const
  {
    return ensureDataForKeyExistsOwning<T>(key, [] { return new T(); });
  }

  template <typename T, typename Factory> T& ensureDataForKeyExistsOwning(auto key, const Factory& factory) const
  {
    auto storage = ensureUserDataStorage();
    auto it = storage->entries.find(key);

    if(it == storage->entries.end() || !it->second)
    {
      auto data = factory();
      auto entry = std::make_unique<UserDataEntry>(data, [](void* p) { delete static_cast<T*>(p); });
      storage->entries[key] = std::move(entry);
      return *data;
    }

    return *static_cast<T*>(it->second->data);
  }

  template <typename T> T& ensureDataForKeyExistsNonOwning(auto key, auto fac) const
  {
    auto storage = ensureUserDataStorage();
    auto it = storage->entries.find(key);

    if(it == storage->entries.end() || !it->second)
    {
      auto data = fac();
      auto entry = std::make_unique<UserDataEntry>(data, [](void* p) { });
      storage->entries[key] = std::move(entry);
      return *static_cast<T*>(data);
    }

    return *static_cast<T*>(it->second->data);
  }

  void clearUserData()
  {
    auto storage = ensureUserDataStorage();
    erase_if(storage->entries, [](const auto& it) { return it.first != c_computationsKey; });
  }

  [[nodiscard]] virtual WidgetType* getHandle() const
  {
    return m_widget;
  }

  template <typename tCB> void doAutorun(tCB&& cb) const
  {
    getComputations().add(
        [cb = std::forward<tCB>(cb)]
        {
          try
          {
            cb();
          }
          catch(const NotSyncedException&)
          {
          }
          catch(const std::exception& e)
          {
            std::cerr << std::format("Computation::execute() failed: {}", e.what()) << std::endl;
          }
          catch(...)
          {
            std::cerr << "Computation::execute() failed." << std::endl;
          }
        });
  }

  [[nodiscard]] UserDataStorage* getUserDataStorage() const
  {
    return static_cast<UserDataStorage*>(lv_obj_get_user_data(m_widget));
  }

 private:
  [[nodiscard]] Reactive::Computations& getComputations() const
  {
    return ensureDataForKeyExistsOwning<Reactive::Computations>(c_computationsKey);
  }

  [[nodiscard]] UserDataStorage* ensureUserDataStorage() const
  {
    auto storage = getUserDataStorage();
    if(!storage)
    {
      storage = new UserDataStorage();
      lv_obj_set_user_data(m_widget, storage);

      lv_obj_add_event_cb(
          m_widget,
          [](lv_event_t* e)
          {
            auto storage
                = static_cast<UserDataStorage*>(lv_obj_get_user_data(static_cast<lv_obj_t*>(lv_event_get_target(e))));
            if(storage)
            {
              delete storage;
              lv_obj_set_user_data(static_cast<lv_obj_t*>(lv_event_get_target(e)), nullptr);
            }
          },
          LV_EVENT_DELETE, nullptr);
    }
    return storage;
  }

  WidgetType* m_widget;
};