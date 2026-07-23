#pragma once

#include "compose/widgets/BaseWidget.h"
#include <cmath>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace Compose
{
  class Widget;

  class DragDropContext
  {
   public:
    using Getter = std::function<std::optional<nlohmann::json>()>;
    using Setter = std::function<void(const nlohmann::json &)>;
    using DragWidgetBuilder = std::function<void(Widget &)>;

    static DragDropContext &get();

    void setSource(lv_obj_t *self, const std::string &type, int offsetX, int offsetY, int rootX, int rootY, const Getter &getter, const DragWidgetBuilder &dragWidgetBuilder);
    void resetSource(lv_obj_t *self);
    void cancelSource(lv_obj_t *self);
    void addTarget(lv_obj_t *self, const std::string &type, const Setter &setter);
    void removeTarget(lv_obj_t *self, const std::string &type);
    void onDragOver(lv_obj_t *dragSource, lv_obj_t *targetProspect, int rootX, int rootY);
    [[nodiscard]] bool isCurrentTarget(lv_obj_t *widget) const;
    [[nodiscard]] bool isDragging() const;
    [[nodiscard]] std::optional<std::string> getDraggedType() const;

   private:
    DragDropContext() = default;

    struct Source
    {
      Source(lv_obj_t *widget, const std::string &type, int offsetX, int offsetY, int rootX, int rootY, const Getter &getter, const DragWidgetBuilder &dragWidgetBuilder);
      ~Source();

      lv_obj_t *m_widget;
      std::string m_type;
      Getter m_getter;
      DragWidgetBuilder m_dragWidgetBuilder;
      lv_obj_t *m_currentTarget = nullptr;
      lv_obj_t *m_dragWidget = nullptr;
      lv_draw_buf_t *m_snapshot = nullptr;
      lv_point_t m_dragPosition;
      std::pair<int, int> m_dragWidgetOffset;
    };

    struct Target
    {
      Target(lv_obj_t *widget, const std::string &type, const Setter &setter);
      lv_obj_t *m_widget;
      std::string m_type;
      Setter m_setter;
    };

    Reactive::Var<std::unique_ptr<Source>> m_source;
    std::vector<std::unique_ptr<Target>> m_targets;
  };

  struct Drag
  {
    explicit Drag(BaseWidget &w);

    struct Begin;
    struct Update;
    struct End;

    using BeginCB = std::function<void()>;
    using UpdateCB = std::function<void(int x, int y)>;
    using EndCB = std::function<void()>;

    struct Data
    {
      Data(lv_obj_t *handle, const BeginCB &beginCB, const UpdateCB &updateCB, const EndCB &endCB);
      ~Data();

      lv_obj_t *m_handle;
      BeginCB m_begin;
      UpdateCB m_update;
      EndCB m_end;
      std::optional<lv_point_t> m_lastPos;
      lv_event_dsc_t *m_pressHandler = nullptr;
      lv_event_dsc_t *m_pressingHandler = nullptr;
      lv_event_dsc_t *m_releaseHandler = nullptr;
      lv_event_dsc_t *m_pressLostHandler = nullptr;
    };

    struct Begin
    {
      explicit Begin(Drag *self);
      void operator<<(const BeginCB &cb) const;

      Drag *m_self;
    };

    struct Update
    {
      explicit Update(Drag *self);
      void operator<<(const UpdateCB &cb) const;

      Drag *m_self;
    };

    struct End
    {
      explicit End(Drag *self);
      void operator<<(const EndCB &cb) const;

      Drag *m_self;
    };

    void operator<<(const std::function<void(Drag *it)> &cb);

    BaseWidget &self;
    BeginCB m_begin = [] { };
    UpdateCB m_update = [](int, int) { };
    EndCB m_end = [] { };
    Begin begin;
    Update update;
    End end;
  };

  struct DragDrop
  {
    explicit DragDrop(BaseWidget &w);

    struct DragDropForContent
    {
      DragDropForContent(lv_obj_t *ownerHandle, std::string type);
      lv_obj_t *ownerHandle;
      std::string type;

      void operator<<(const std::function<void(DragDropForContent *it)> &cb);

      struct Source
      {
        using Getter = std::function<std::optional<nlohmann::json>()>;

        struct StartAxis
        {
          enum Direction
          {
            Any,
            Horizontal,
            Vertical
          } it;

          static constexpr StartAxis ANY()
          {
            return { Any };
          }

          static constexpr StartAxis HORIZONTAL()
          {
            return { Horizontal };
          }

          static constexpr StartAxis VERTICAL()
          {
            return { Vertical };
          }
        };

        explicit Source(DragDropForContent *self);

        struct Data
        {
          enum class StartDecision
          {
            Undecided,
            AllowDrag,
            BlockDrag
          };

          Data(lv_obj_t *handle, std::string type, const Getter &getter, const DragDropContext::DragWidgetBuilder &dragWidgetBuilder, StartAxis::Direction startAxis);
          ~Data();

          lv_obj_t *m_handle;
          std::string m_type;
          Getter m_getter;
          DragDropContext::DragWidgetBuilder m_dragWidgetBuilder;
          StartAxis::Direction m_startAxis = StartAxis::Any;
          StartDecision m_startDecision = StartDecision::Undecided;
          std::optional<lv_point_t> m_startPos;
          lv_point_t m_offset;
          std::vector<lv_obj_t *> m_suppressedScrollables;
          lv_event_dsc_t *m_pressHandler = nullptr;
          lv_event_dsc_t *m_pressingHandler = nullptr;
          lv_event_dsc_t *m_releaseHandler = nullptr;
          lv_event_dsc_t *m_pressLostHandler = nullptr;
        };

        void setModifier(StartAxis axis);
        void operator<<(const Getter &cb) const;

        DragDropForContent *self;
        StartAxis::Direction m_startAxis = StartAxis::Any;
      };

      struct Target
      {
        using Setter = std::function<void(const nlohmann::json &)>;

        explicit Target(DragDropForContent *self);

        struct Data
        {
          Data(lv_obj_t *handle, std::string type);
          ~Data();

          void setSetter(const Setter &setter);

          lv_obj_t *m_handle;
          std::string m_type;
          std::shared_ptr<Setter> m_setter = std::make_shared<Setter>([](const nlohmann::json &) { });
        };

        void operator<<(const Setter &cb);

        DragDropForContent *self;
      };

      struct BuildDragWidget
      {
        explicit BuildDragWidget(DragDropForContent *self);

        using Builder = std::function<void(Widget &)>;

        void operator<<(const Builder &cb);
        [[nodiscard]] const Builder &get() const;

        DragDropForContent *self;

       private:
        Builder m_builder;
      } buildDragWidget;

      std::unique_ptr<Source> source;
      std::unique_ptr<Target> target;
      Source::StartAxis m_startAxis = Source::StartAxis::ANY();
    };

    DragDropForContent &operator()(const std::string &type, DragDropForContent::Source::StartAxis startAxis = DragDropForContent::Source::StartAxis::ANY());

    using tContentMap = std::map<std::string, std::unique_ptr<DragDropForContent>>;

    BaseWidget &self;
    std::shared_ptr<tContentMap> m_dragDropForContent = std::make_shared<tContentMap>();
  };
}

#define DRAG() it.drag << [=](Compose::Drag * it)
#define DRAG_BEGIN() it->begin << [=]
#define DRAG_UPDATE it->update << [=]
#define DRAG_END() it->end << [=]
#define DRAG_DROP(type) it.dragDrop(type) << [=](Compose::DragDrop::DragDropForContent * it)
#define DRAG_DROP_VERTICAL(type) it.dragDrop(type, Compose::DragDrop::DragDropForContent::Source::StartAxis::VERTICAL()) << [=](Compose::DragDrop::DragDropForContent * it)
#define DRAG_SOURCE() (*it->source) << [=]
#define DROP_TARGET (*it->target) << [=]
#define DRAG_PROXY_WIDGET() it->buildDragWidget << [=](Compose::Widget & it)
