#pragma once
#include "src/misc/lv_event_private.h"

#include <nltools/logging/Log.h>
#include <reactive/Deferrer.h>
#include <memory>

namespace Gtk
{
  class Widget;
}

namespace Compose
{
  template <typename tHandle> class LeftClick : public std::enable_shared_from_this<LeftClick<tHandle>>
  {
   public:
    tHandle& widget;
    std::function<void()> m_callback;

    explicit LeftClick(tHandle& widget)
        : widget(widget)
    {
    }

    template <typename tCB> void operator<<(const tCB& cb)
    {
      m_callback = cb;
      auto handle = widget.getHandle();
      lv_obj_add_event_cb(
          handle,
          [](lv_event_t* e)
          {
            auto user_data = static_cast<LeftClick*>(lv_event_get_user_data(e));

            if(lv_event_get_code(e) == LV_EVENT_CLICKED)
            {
              if(user_data->m_callback)
              {
                user_data->m_callback();
              }
            }
          },
          LV_EVENT_ALL, this);

      struct SharedPtrWrapper
      {
        std::shared_ptr<LeftClick> handler_ptr;
      };

      auto* wrapper = new SharedPtrWrapper(LeftClick::shared_from_this());

      lv_obj_add_event_cb(
          handle,
          [](lv_event_t* e)
          {
            if(lv_event_get_code(e) == LV_EVENT_DELETE)
            {
              delete static_cast<SharedPtrWrapper*>(lv_event_get_user_data(e));
            }
          },
          LV_EVENT_DELETE, wrapper);
    }
  };
}

#define LEFT_CLICK() *it.leftClickHandler << [=]