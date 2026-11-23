#include "LVGLFixture.h"
#include "compose/widgets/container/List.h"
#include "src/core/lv_obj_draw_private.h"

#include <catch2/catch_all.hpp>
#include <reactive/Var.h>
#include <vector>

using Compose::Test::LVGLFixture;

namespace
{
  struct ItemAutoruns
  {
    Reactive::Var<int> observed { 0 };
    std::vector<size_t> ranFor;

    Compose::BasicItemBuilder<size_t> builder()
    {
      return [this](Compose::Widget &it, const size_t &item)
      {
        it.doAutorun(
            [this, item]
            {
              observed.get();
              ranFor.push_back(item);
            });
      };
    }

    std::vector<size_t> runsAfterChange()
    {
      ranFor.clear();
      observed = observed.peek() + 1;
      return ranFor;
    }
  };
}

TEST_CASE_METHOD(LVGLFixture, "A reused list item container only runs the autoruns of its current item", "[List]")
{
  ItemAutoruns autoruns;
  Compose::Widget root(m_root);
  Compose::BasicListItemContainer<size_t> container(root, Compose::Axis::Vertical);

  container.setChildId(autoruns.builder(), 1);
  container.setChildId(autoruns.builder(), 2);

  CHECK(autoruns.runsAfterChange() == std::vector<size_t> { 2 });

  lv_obj_delete(container.getHandle());
}

TEST_CASE_METHOD(LVGLFixture, "A list item container leaves room for what its item draws beyond itself", "[List]")
{
  constexpr int growth = 2;
  Compose::Widget root(m_root);
  Compose::BasicListItemContainer<size_t> container(root, Compose::Axis::Vertical);

  container.setChildId([](Compose::Widget &it, const size_t &) { it.setModifier(Compose::TransformSize { .width = growth, .height = growth }); },
                       1);

  CHECK(lv_obj_has_flag(container.getHandle(), LV_OBJ_FLAG_OVERFLOW_VISIBLE));
  CHECK(lv_obj_get_ext_draw_size(container.getHandle()) == growth);
}
