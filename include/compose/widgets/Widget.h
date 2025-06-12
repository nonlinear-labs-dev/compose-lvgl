#pragma once
#include "BaseWidget.h"
#include "compose/modifiers/Modifiers.h"
#include "handler/Clicks.h"
#include "compose/modifiers/RoundedCorner.h"
#include "compose/modifiers/Padding.h"
#include "compose/modifiers/Hidden.h"
#include "nltools-2/Uuid.h"
#include "nltools/Assert.h"

namespace Gtk
{
  class Widget;
}

namespace Compose
{
  template <typename tType> class Widget : public BaseWidget<tType>
  {
   protected:
    void applyCssStyle(const char *key, const std::string &css) const
    {
      try
      {
        super::template ensureDataForKeyExistsNonOwning<Gtk::CssProvider>(
            key,
            [handle = getHandle()]
            {
              auto css_provider = Gtk::CssProvider::create();
              handle->get_style_context()->add_provider(css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
              return css_provider.get();
            })
            .load_from_data(css);
      }
      catch(Gtk::CssProviderError e)
      {
        nltools::Log::error(__PRETTY_FUNCTION__, e.what());
      }
    }

   public:
    using super = BaseWidget<tType>;
    using super::clearConnections;
    using super::ensureDataForKeyExistsNonOwning;
    using super::ensureDataForKeyExistsOwning;
    using super::getData;
    using super::getHandle;

    using WidgetType = tType;
    using AutorunCB = std::function<void(WidgetType &&)>;

    explicit Widget(WidgetType *w)
        : BaseWidget<tType>(w)
    {
    }

    int getWidth() const
    {
      return BaseWidget<tType>::getHandle()->get_allocation().get_width();
    }

    int getHeight() const
    {
      return BaseWidget<tType>::getHandle()->get_allocation().get_height();
    }

    ~Widget() override = default;

    template <typename T> T &&add(T &&toAdd)
    {
      getHandle()->add(*toAdd.getHandle());
      return std::forward<T>(toAdd);
    }

    virtual void clear()
    {
    }

    virtual void showAll()
    {
      getHandle()->show_all();
    }

    void setModifier(Orientation o) const
    {
      getHandle()->set_orientation(to<Gtk::Orientation>(o.it));
    }

    void setModifier(Expand e) const
    {
      getHandle()->set_vexpand(e.vertical);
      getHandle()->set_hexpand(e.horizontal);
    }

    void setModifier(Homogeneous h) const
    {
      getHandle()->set_homogeneous(h.it);
    }

    void setModifier(RowHomogeneous h) const
    {
      getHandle()->set_row_homogeneous(h.it);
    }

    void setModifier(ColumnHomogeneous h) const
    {
      getHandle()->set_column_homogeneous(h.it);
    };

    void setModifier(StyleSheet s) const
    {
      auto context = super::getHandle()->get_style_context();
      auto classes = context->list_classes();

      for(const auto &class_name : classes)
      {
        context->remove_class(class_name);
      }

      context->add_class(s.className);
    }

    void setModifier(HAlign o) const
    {
      getHandle()->set_halign(to<Gtk::Align>(o.it));
    }

    void setModifier(VAlign o) const
    {
      getHandle()->set_valign(to<Gtk::Align>(o.it));
    }

    void setModifier(BackgroundColor col) const
    {
      Gdk::RGBA color {};
      color.set_rgba(col.r / 255., col.g / 255., col.b / 255., col.a);
      getHandle()->override_background_color(color);
    }

    virtual void setModifier(PrimaryColor col) const
    {
      Gdk::RGBA color {};
      color.set_rgba(col.r / 255., col.g / 255., col.b / 255., col.a);
      getHandle()->override_color(color);
    }

    void setModifier(MinSize s) const
    {
      getHandle()->set_size_request(s.w, s.h);
    }

    void setModifier(Margin m) const
    {
      getHandle()->set_margin_top(m.top);
      getHandle()->set_margin_bottom(m.bottom);
      getHandle()->set_margin_left(m.left);
      getHandle()->set_margin_right(m.right);
    }

    LeftClick<Widget> leftClickHandler { *this };
    RightClick<Widget> rightClickHandler { *this };

    void setModifier(RoundedCorner corner) const
    {
      applyCssStyle(RoundedCorner::key, nltools::string::concat("* { ", corner.cssRuleText(), " }"));
    }

    void setModifier(Padding padding) const
    {
      applyCssStyle(Padding::key, nltools::string::concat("* { ", padding.cssRuleText(), " }"));
    }

    void setModifier(Hidden hidden) const
    {
      getHandle()->set_visible(!hidden.it);
    }

    void setModifier(Position pos) const
    {
      if(auto handle = getHandle())
      {
        if(auto parent = getHandle()->get_parent())
        {
          if(const auto fixed = dynamic_cast<Gtk::Fixed *>(parent))
          {
            fixed->move(*handle, pos.x, pos.y);
          }
          else
          {
            nltools_assertNotReached();
          }
        }
      }
    }
  };

  template <typename T>
  concept IsWidget = requires { typename T::WidgetType; };

  template <typename ComposeWidget, typename tCB>
    requires IsWidget<ComposeWidget>
  void operator<<(ComposeWidget &&lhs, tCB &&cb)
  {
    using tComposeWidgetDecayed = std::remove_reference_t<ComposeWidget>;

    lhs.doAutorun(
        [cb = std::forward<tCB>(cb), w = lhs.getHandle()]
        {
          tComposeWidgetDecayed wrapper(w);
          wrapper.clear();
          wrapper.clearConnections();

          cb(tComposeWidgetDecayed(w));
          wrapper.showAll();
        });
  }
}
