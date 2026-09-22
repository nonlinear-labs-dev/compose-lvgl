#include "compose/widgets/Widget.h"
#include "compose/widgets/handler/DragDrop.h"
#include "lvgl.h"

#include <catch2/catch_all.hpp>

namespace
{
  constexpr auto c_type = "preset";
  constexpr auto c_key = "DragDropSource_preset";
  constexpr auto c_payloadKey = "uuid";

  using SourceData = Compose::DragDrop::DragDropForContent::Source::Data;

  struct LVGLFixture
  {
    LVGLFixture()
    {
      if(!lv_is_initialized())
        lv_init();

      m_display = lv_display_create(800, 480);
      m_root = lv_obj_create(nullptr);
    }

    ~LVGLFixture()
    {
      lv_obj_delete(m_root);
      lv_display_delete(m_display);
    }

    LVGLFixture(const LVGLFixture &) = delete;
    LVGLFixture &operator=(const LVGLFixture &) = delete;

    lv_display_t *m_display = nullptr;
    lv_obj_t *m_root = nullptr;
  };

  void bindDragSource(Compose::Widget &widget, const std::string &payload)
  {
    widget.dragDrop(c_type) << [payload](Compose::DragDrop::DragDropForContent *it)
    { (*it->source) << [payload] { return nlohmann::json { { c_payloadKey, payload } }; }; };
  }

  std::string draggedPayloadOf(const Compose::Widget &widget)
  {
    auto *data = widget.getData<SourceData>(c_key);
    REQUIRE(data != nullptr);

    const auto content = data->m_getter();
    REQUIRE(content.has_value());

    return content->at(c_payloadKey).get<std::string>();
  }
}

TEST_CASE_METHOD(LVGLFixture, "Rebinding a drag source replaces its payload", "[DragDrop]")
{
  Compose::Widget widget(lv_obj_create(m_root));

  bindDragSource(widget, "first");
  CHECK(draggedPayloadOf(widget) == "first");

  bindDragSource(widget, "second");
  CHECK(draggedPayloadOf(widget) == "second");
}
