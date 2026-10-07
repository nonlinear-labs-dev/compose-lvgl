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

  void bindDropTarget(Compose::Widget &widget)
  {
    widget.dragDrop(c_type) << [](Compose::DragDrop::DragDropForContent *it)
    { (*it->target) << [](const nlohmann::json &) { }; };
  }

  lv_obj_t *setArea(lv_obj_t *obj, const lv_area_t &area)
  {
    lv_obj_set_pos(obj, area.x1, area.y1);
    lv_obj_set_size(obj, lv_area_get_width(&area), lv_area_get_height(&area));
    return obj;
  }

  lv_obj_t *placeAt(lv_obj_t *parent, const lv_area_t &area)
  {
    return setArea(lv_obj_create(parent), area);
  }

  lv_obj_t *placeUnstyledAt(lv_obj_t *parent, const lv_area_t &area)
  {
    auto *obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    return setArea(obj, area);
  }

  class TestFinger
  {
   public:
    TestFinger()
        : m_indev(lv_indev_create())
    {
      lv_indev_set_type(m_indev, LV_INDEV_TYPE_POINTER);
      lv_indev_set_user_data(m_indev, this);
      lv_indev_set_read_cb(m_indev, read);
    }

    ~TestFinger()
    {
      lv_indev_delete(m_indev);
    }

    TestFinger(const TestFinger &) = delete;
    TestFinger &operator=(const TestFinger &) = delete;

    void pressAt(const lv_point_t point)
    {
      m_state = LV_INDEV_STATE_PRESSED;
      moveTo(point);
    }

    void moveTo(const lv_point_t point)
    {
      m_point = point;
      lv_indev_read(m_indev);
    }

   private:
    static void read(lv_indev_t *indev, lv_indev_data_t *data)
    {
      const auto *self = static_cast<TestFinger *>(lv_indev_get_user_data(indev));
      data->point = self->m_point;
      data->state = self->m_state;
    }

    lv_indev_t *m_indev;
    lv_point_t m_point { .x = 0, .y = 0 };
    lv_indev_state_t m_state = LV_INDEV_STATE_RELEASED;
  };
}

TEST_CASE_METHOD(LVGLFixture, "Rebinding a drag source replaces its payload", "[DragDrop]")
{
  Compose::Widget widget(lv_obj_create(m_root));

  bindDragSource(widget, "first");
  CHECK(draggedPayloadOf(widget) == "first");

  bindDragSource(widget, "second");
  CHECK(draggedPayloadOf(widget) == "second");
}

TEST_CASE_METHOD(LVGLFixture, "Content scrolled out of its container does not hide a drop target", "[DragDrop]")
{
  lv_screen_load(m_root);

  Compose::Widget target(placeAt(m_root, { .x1 = 0, .y1 = 0, .x2 = 799, .y2 = 99 }));
  auto *scroller = placeAt(m_root, { .x1 = 0, .y1 = 200, .x2 = 799, .y2 = 399 });
  placeAt(scroller, { .x1 = 0, .y1 = 0, .x2 = 799, .y2 = 1999 });
  Compose::Widget source(placeAt(m_root, { .x1 = 0, .y1 = 420, .x2 = 199, .y2 = 469 }));

  bindDropTarget(target);
  bindDragSource(source, "dragged");

  lv_obj_update_layout(m_root);
  lv_obj_scroll_to_y(scroller, 1000, LV_ANIM_OFF);

  TestFinger finger;
  finger.pressAt({ .x = 100, .y = 445 });
  finger.moveTo({ .x = 100, .y = 50 });

  CHECK(target.isCurrentDropTarget());
}

TEST_CASE_METHOD(LVGLFixture, "A drop target overflowing a parent with visible overflow stays reachable", "[DragDrop]")
{
  lv_screen_load(m_root);

  auto *parent = placeUnstyledAt(m_root, { .x1 = 100, .y1 = 100, .x2 = 299, .y2 = 199 });
  lv_obj_add_flag(parent, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
  Compose::Widget target(placeUnstyledAt(parent, { .x1 = 150, .y1 = 50, .x2 = 249, .y2 = 149 }));
  Compose::Widget source(placeAt(m_root, { .x1 = 0, .y1 = 420, .x2 = 199, .y2 = 469 }));

  bindDropTarget(target);
  bindDragSource(source, "dragged");
  lv_obj_update_layout(m_root);

  TestFinger finger;
  finger.pressAt({ .x = 100, .y = 445 });
  finger.moveTo({ .x = 320, .y = 230 });

  CHECK(target.isCurrentDropTarget());
}
