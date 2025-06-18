#include <compose/widgets/container/Container.h>

namespace Compose
{
  Container::Container(WidgetType *handle)
      : Widget(handle)
  {
  }

  void Container::setModifier(Spacing r) const
  {
    // getHandle()->set_spacing(r.it);
  }

  void Container::setModifier(Orientation r) const
  {
    switch(r.it)
    {
      case OrientationEnum::HORIZONTAL:
        lv_obj_set_flex_flow(getHandle(), LV_FLEX_FLOW_ROW);
        break;
      case OrientationEnum::VERTICAL:
        lv_obj_set_flex_flow(getHandle(), LV_FLEX_FLOW_COLUMN);
        break;
    }
  }

  void Container::setModifier(FixedSize r) const
  {
    // auto handle = getHandle();
    // if constexpr(requires { handle->set_resize_mode(Gtk::RESIZE_QUEUE); })
    //   handle->set_resize_mode(Gtk::RESIZE_QUEUE);
    //
    // if(r.w != -1 || r.h != -1)
    //   handle->set_size_request(r.w, r.h);
    //
    // auto connection = getHandle()->signal_size_allocate().connect(
    //     [handle](Gtk::Allocation &allocation)
    //     {
    //       int w = 0;
    //       int h = 0;
    //       handle->get_size_request(w, h);
    //       if(w != -1)
    //         allocation.set_width(w);
    //       if(h != -1)
    //         allocation.set_height(h);
    //       handle->set_allocation(allocation);
    //     });
    // addConnection(connection);
  }
}