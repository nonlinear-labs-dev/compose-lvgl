#pragma once
#include "compose/modifiers/Color.h"
#include "compose/modifiers/Rect.h"
#include "compose/modifiers/RoundedCorner.h"
#include "compose/modifiers/Text.h"
#include "compose/modifiers/Font.h"
#include "compose/modifiers/Modifiers.h"
#include "src/draw/lv_draw.h"
#include "src/misc/lv_types.h"

#include <tuple>
#include <vector>
#include <memory>
#include <optional>
#include "compose/widgets/Label.h"

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

    struct Bitmap
    {
      int width {};
      int height {};
      int stride {};

      const uint8_t *start;
    };

    virtual void drawLine(StrokeStyle style, Point p1, Point p2) = 0;
    virtual void strokeRect(StrokeStyle style, Rect r) = 0;
    virtual void fillRect(Color color, Rect r) = 0;
    virtual void fillRoundedRect(Color color, Rect r, RoundedCorner rc) = 0;
    virtual void fillPolygon(StrokeStyle stroke, Color fill, std::vector<Point> points) = 0;
    virtual void fillArc(Color color, Point position, float radius, int width, double startAngle, double sweep) = 0;
    virtual void drawText(Text t, Font f, Rect r, Color c, TextAlign ta) = 0;
    virtual void putBitmap(const Bitmap &image, Point p, std::optional<Color> colorOverride = std::nullopt) = 0;
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
    void fillArc(Color color, Point position, float radius, int width, double startAngle, double sweep) override;
    void drawText(Text t, Font f, Rect r, Color c, TextAlign ta) override;
    void putBitmap(const Bitmap &image, Point p, std::optional<Color> colorOverride = std::nullopt) override;

   private:
    lv_layer_t m_layer;
    tCanvas m_canvas;
  };

  extern std::unique_ptr<FontStorage> s_fontStorage;
};
