#pragma once
#include "compose/modifiers/Color.h"
#include "compose/modifiers/Rect.h"
#include "compose/modifiers/RoundedCorner.h"
#include "src/draw/lv_draw.h"
#include "src/misc/lv_types.h"

#include <tuple>
#include <vector>

namespace Compose
{
  class DrawContext
  {
   public:
    virtual ~DrawContext() = default;

    struct StrokeStyle
    {
      int width {};
      Color color {};
    };

    struct FillStyle
    {
      Color color {};
    };

    virtual void drawLine(StrokeStyle style, Point p1, Point p2) = 0;
    virtual void strokeRect(StrokeStyle style, Rect r) = 0;
    virtual void fillRect(Color color, Rect r) = 0;
    virtual void fillRoundedRect(Color color, Rect r, RoundedCorner rc) = 0;
    virtual void fillPolygon(StrokeStyle stroke, Color fill, std::vector<Point> points) = 0;
  };

  class LVGLDrawContext : public DrawContext
  {
   public:
    using tCanvas = lv_obj_t *;

    explicit LVGLDrawContext(tCanvas ctx);
    ~LVGLDrawContext() override;
    void drawLine(StrokeStyle style, Point p1, Point p2) override;
    void strokeRect(StrokeStyle style, Rect rect) override;
    void fillRect(Color color, Rect rect) override;
    void fillRoundedRect(Color color, Rect r, RoundedCorner rc) override;
    void fillPolygon(StrokeStyle stroke, Color fill, std::vector<Point> points) override;

   private:
    lv_layer_t m_layer;
    tCanvas m_canvas;
  };
};
