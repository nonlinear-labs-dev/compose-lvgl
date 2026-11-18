#include "LVGLFixture.h"
#include "compose/widgets/Widget.h"
#include "compose/widgets/handler/DragDrop.h"

#include <catch2/catch_all.hpp>
#include <string>

namespace
{
  using Compose::Position;

  struct Recorder
  {
    auto record(const std::string &name)
    {
      return [this, name](auto &&...) { lastCalled = name; };
    }

    auto recordEvent(const std::string &name)
    {
      return [this, name](Position)
      {
        lastCalled = name;
        return false;
      };
    }

    std::string lastCalled;
  };

  template <lv_event_code_t Code> std::string lastCalledAfterRebind(lv_obj_t *handle, Compose::EventHandler<Code> Compose::Widget::*handler)
  {
    Recorder recorder;
    Compose::Widget(handle).*handler << recorder.recordEvent("first");

    Compose::Widget rebound(handle);
    rebound.*handler << recorder.recordEvent("second");

    const auto *data = rebound.getData<typename Compose::EventHandler<Code>::EventData>((rebound.*handler).m_key);
    REQUIRE(data != nullptr);
    data->m_callback(Position {});
    return recorder.lastCalled;
  }

  void bindTouch(lv_obj_t *handle, Recorder &recorder, const std::string &name)
  {
    Compose::Widget(handle).touch << [&recorder, name](Compose::Touch *it)
    {
      it->begin << recorder.record(name + " begin");
      it->update << recorder.record(name + " update");
      it->end << recorder.record(name + " end");
    };
  }

  void bindDrag(lv_obj_t *handle, Recorder &recorder, const std::string &name)
  {
    Compose::Widget(handle).drag << [&recorder, name](Compose::Drag *it)
    {
      it->begin << recorder.record(name + " begin");
      it->update << recorder.record(name + " update");
      it->end << recorder.record(name + " end");
    };
  }
}

TEST_CASE_METHOD(Compose::Test::LVGLFixture, "Rebinding an event handler replaces its callback", "[Handlers]")
{
  auto *handle = lv_obj_create(m_root);

  CHECK(lastCalledAfterRebind(handle, &Compose::Widget::pressed) == "second");
  CHECK(lastCalledAfterRebind(handle, &Compose::Widget::released) == "second");
  CHECK(lastCalledAfterRebind(handle, &Compose::Widget::clicked) == "second");
  CHECK(lastCalledAfterRebind(handle, &Compose::Widget::longClick) == "second");
}

TEST_CASE_METHOD(Compose::Test::LVGLFixture, "Rebinding a state change handler replaces its callback", "[Handlers]")
{
  auto *handle = lv_obj_create(m_root);
  Recorder recorder;

  Compose::Widget(handle).stateChange << recorder.record("first");
  Compose::Widget(handle).stateChange << recorder.record("second");

  const auto *data = Compose::Widget(handle).getData<Compose::StateChange::StateChangeData>(BaseWidget::c_stateChangeKey);
  REQUIRE(data != nullptr);
  data->m_callback(true);
  CHECK(recorder.lastCalled == "second");
}

TEST_CASE_METHOD(Compose::Test::LVGLFixture, "Rebinding a timer tick replaces its callback", "[Handlers]")
{
  auto *handle = lv_obj_create(m_root);
  Recorder recorder;

  Compose::Widget(handle).timerTick << recorder.record("first");
  Compose::Widget(handle).timerTick << recorder.record("second");

  const auto *data = Compose::Widget(handle).getData<Compose::TimerTick::TimerData>("TimerData");
  REQUIRE(data != nullptr);
  data->cb();
  CHECK(recorder.lastCalled == "second");
}

TEST_CASE_METHOD(Compose::Test::LVGLFixture, "Rebinding a touch handler replaces its callbacks", "[Handlers]")
{
  auto *handle = lv_obj_create(m_root);
  Recorder recorder;

  bindTouch(handle, recorder, "first");
  bindTouch(handle, recorder, "second");

  const auto *data = Compose::Widget(handle).getData<Compose::Touch::Data>("TouchData");
  REQUIRE(data != nullptr);

  data->m_begin(0, Position {}, 1);
  CHECK(recorder.lastCalled == "second begin");
  data->m_update(0, Position {}, 1);
  CHECK(recorder.lastCalled == "second update");
  data->m_end(0, Position {}, 0);
  CHECK(recorder.lastCalled == "second end");
}

TEST_CASE_METHOD(Compose::Test::LVGLFixture, "Rebinding a drag handler replaces its callbacks", "[Handlers]")
{
  auto *handle = lv_obj_create(m_root);
  Recorder recorder;

  bindDrag(handle, recorder, "first");
  bindDrag(handle, recorder, "second");

  const auto *data = Compose::Widget(handle).getData<Compose::Drag::Data>("DragHandlerData");
  REQUIRE(data != nullptr);

  data->m_begin();
  CHECK(recorder.lastCalled == "second begin");
  data->m_update(0, 0);
  CHECK(recorder.lastCalled == "second update");
  data->m_end();
  CHECK(recorder.lastCalled == "second end");
}
