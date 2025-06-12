#include <compose/widgets/container/Fixed.h>

namespace Compose
{
  Fixed::Fixed(WidgetType *handle)
      : Widget(handle)
  {
  }

  void Fixed::clear()
  {
    for(const auto c : getHandle()->get_children())
    {
      getHandle()->remove(*c);
      delete c;
    }
  }

  void Fixed::setModifier(FixedSize r) const
  {
    auto handle = getHandle();
    if constexpr(requires { handle->set_resize_mode(Gtk::RESIZE_QUEUE); })
      handle->set_resize_mode(Gtk::RESIZE_QUEUE);

    if(r.w != -1 || r.h != -1)
      handle->set_size_request(r.w, r.h);

    getHandle()->signal_size_allocate().connect(
        [handle](Gtk::Allocation &allocation)
        {
          int w = 0;
          int h = 0;
          handle->get_size_request(w, h);
          if(w != -1)
            allocation.set_width(w);
          if(h != -1)
            allocation.set_height(h);
          handle->set_allocation(allocation);
        });
  }
}
