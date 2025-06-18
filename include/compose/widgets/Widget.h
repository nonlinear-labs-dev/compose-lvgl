#pragma once
#include "BaseWidget.h"
#include "compose/modifiers/Modifiers.h"
#include "handler/Clicks.h"
#include "compose/modifiers/RoundedCorner.h"
#include "compose/modifiers/Padding.h"
#include "compose/modifiers/Hidden.h"
#include "nltools-2/Uuid.h"
#include "nltools/Assert.h"
#include "src/core/lv_obj_private.h"

namespace Compose
{
  class Widget : public BaseWidget
  {
   public:
    using WidgetType = lv_obj_t;
    using super = BaseWidget;
    using super::ensureDataForKeyExistsNonOwning;
    using super::ensureDataForKeyExistsOwning;
    using super::getData;
    using super::getHandle;
    using AutorunCB = std::function<void(WidgetType &&)>;

    explicit Widget(WidgetType *w)
        : BaseWidget(w)
    {
    }

    int getWidth() const
    {
      return lv_obj_get_width(getHandle());
    }

    int getHeight() const
    {
      return lv_obj_get_height(getHandle());
    }

    ~Widget() override = default;

    // template <typename T> T &&add(T &&toAdd)
    // {
    //   // getHandle()->add(*toAdd.getHandle());
    //   // return std::forward<T>(toAdd);
    // }

    virtual void clear()
    {
      lv_obj_clean(getHandle());
    }

    void setModifier(Orientation o) const
    {
      // getHandle()->set_orientation(to<Gtk::Orientation>(o.it));
    }

    void setModifier(Expand e) const
    {
      //getHandle()->set_vexpand(e.vertical);
      // getHandle()->set_hexpand(e.horizontal);
    }

    void setModifier(Homogeneous h) const
    {
      // getHandle()->set_homogeneous(h.it);
    }

    void setModifier(RowHomogeneous h) const
    {
      // getHandle()->set_row_homogeneous(h.it);
    }

    void setModifier(ColumnHomogeneous h) const {
      // getHandle()->set_column_homogeneous(h.it);
    };

    void setModifier(HAlign o) const
    {
      // getHandle()->set_halign(to<Gtk::Align>(o.it));
    }

    void setModifier(VAlign o) const
    {
      // getHandle()->set_valign(to<Gtk::Align>(o.it));
    }

    void setModifier(BackgroundColor col) const
    {
      const auto opacity = static_cast<unsigned char>(col.a * 255.0);
      lv_obj_set_style_bg_color(getHandle(),
                                lv_color_t {
                                    .blue = col.b,
                                    .green = col.g,
                                    .red = col.r,
                                },
                                LV_PART_MAIN);
      lv_obj_set_style_bg_opa(getHandle(), opacity, LV_PART_MAIN);
    }

    void setModifier(MinSize s) const
    {
      // getHandle()->set_size_request(s.w, s.h);
    }

    void setModifier(Margin m) const
    {
      // getHandle()->set_margin_top(m.top);
      // getHandle()->set_margin_bottom(m.bottom);
      // getHandle()->set_margin_left(m.left);
      // getHandle()->set_margin_right(m.right);
    }

    LeftClick<Widget> leftClickHandler { *this };
    RightClick<Widget> rightClickHandler { *this };

    void setModifier(RoundedCorner corner) const
    {
      // applyCssStyle(RoundedCorner::key, nltools::string::concat("* { ", corner.cssRuleText(), " }"));
    }

    void setModifier(Padding padding) const
    {
      // applyCssStyle(Padding::key, nltools::string::concat("* { ", padding.cssRuleText(), " }"));
    }

    void setModifier(Hidden hidden) const
    {
      // getHandle()->set_visible(!hidden.it);
    }

    void setModifier(Position pos) const
    {
      // if(auto handle = getHandle())
      // {
      //   if(auto parent = getHandle()->get_parent())
      //   {
      //     if(const auto fixed = dynamic_cast<Gtk::Fixed *>(parent))
      //     {
      //       fixed->move(*handle, pos.x, pos.y);
      //     }
      //     else
      //     {
      //       nltools_assertNotReached();
      //     }
      //   }
      // }
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
          cb(tComposeWidgetDecayed(w));
        });
  }
}
