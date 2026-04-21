#include <compose/widgets/DrawContext.h>
#include "src/draw/lv_draw.h"
#include "src/misc/lv_color.h"
#include "src/misc/lv_area.h"
#include <src/widgets/canvas/lv_canvas.h>
#include "src/draw/lv_draw_vector.h"
#include <memory>
#include <algorithm>
#include <cmath>
#include "compose/widgets/Label.h"
#include <compose/FreeTypeFont.h>
#include "src/widgets/canvas/lv_canvas_private.h"
#include "compose/widgets/LabelShared.h"

namespace Compose
{
  std::unique_ptr<FontStorage> s_fontStorage = nullptr;

  LVGLDrawContext::LVGLDrawContext(tCanvas &ctx)
      : m_layer { }
      , m_canvas(ctx)
  {
    lv_canvas_init_layer(&m_canvas, &m_layer);
    lv_canvas_fill_bg(&m_canvas, lv_color_black(), LV_OPA_0);
  }

  LVGLDrawContext::~LVGLDrawContext()
  {
    lv_canvas_finish_layer(&m_canvas, &m_layer);
  }

  void LVGLDrawContext::drawLine(const StrokeStyle style, const Point p1, const Point p2)
  {
    drawLine(style, p1, p2, std::nullopt, std::nullopt);
  }

  void LVGLDrawContext::drawLine(StrokeStyle style, Point p1, Point p2, std::optional<LineDashOptions> dash,
                                 std::optional<RoundedEnds> ends)
  {
    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    line_dsc.p1.x = p1.x;
    line_dsc.p1.y = p1.y;
    line_dsc.p2.x = p2.x;
    line_dsc.p2.y = p2.y;
    line_dsc.color = lv_color_make(style.color.r, style.color.g, style.color.b);
    line_dsc.width = style.width;
    line_dsc.opa = static_cast<lv_opa_t>(style.color.a * 255.0);

    if(dash.has_value())
    {
      line_dsc.dash_gap = dash.value().dashGap;
      line_dsc.dash_width = dash.value().dashWidth;
    }

    if(ends.has_value())
    {
      line_dsc.round_end = ends.value().end;
      line_dsc.round_start = ends.value().start;
    }

    lv_draw_line(&m_layer, &line_dsc);
  }

  void LVGLDrawContext::drawLineAA(StrokeStyle style, PointF p1, PointF p2, std::optional<LineDashOptions> dash,
                                   std::optional<RoundedEnds> ends)
  {
    using tVectorDscPtr = std::unique_ptr<lv_vector_dsc_t, decltype(&lv_vector_dsc_delete)>;
    using tVectorPathPtr = std::unique_ptr<lv_vector_path_t, decltype(&lv_vector_path_delete)>;

    auto dsc = tVectorDscPtr(lv_vector_dsc_create(&m_layer), &lv_vector_dsc_delete);
    auto path = tVectorPathPtr(lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM), &lv_vector_path_delete);

    if(!dsc || !path)
      return;

    // --- geometry (still int → float, but keeps API compatible)
    lv_fpoint_t p0 = { static_cast<float>(p1.x), static_cast<float>(p1.y) };
    lv_fpoint_t p1f = { static_cast<float>(p2.x), static_cast<float>(p2.y) };

    lv_vector_path_move_to(path.get(), &p0);
    lv_vector_path_line_to(path.get(), &p1f);

    // --- stroke setup
    lv_vector_dsc_set_fill_opa(dsc.get(), LV_OPA_TRANSP);

    lv_vector_dsc_set_stroke_color(dsc.get(), lv_color_make(style.color.r, style.color.g, style.color.b));

    lv_vector_dsc_set_stroke_opa(dsc.get(), static_cast<lv_opa_t>(style.color.a * 255.0f));

    lv_vector_dsc_set_stroke_width(dsc.get(), static_cast<float>(style.width));

    lv_vector_dsc_set_stroke_join(dsc.get(), LV_VECTOR_STROKE_JOIN_ROUND);

    // --- rounded ends handling
    lv_vector_stroke_cap_t cap = LV_VECTOR_STROKE_CAP_BUTT;

    if(ends.has_value())
    {
      if(ends->start && ends->end)
        cap = LV_VECTOR_STROKE_CAP_ROUND;
      else
        cap = LV_VECTOR_STROKE_CAP_BUTT;  // asymmetric handled below
    }

    lv_vector_dsc_set_stroke_cap(dsc.get(), cap);

    // --- dash support
    if(dash.has_value())
    {
      float dashes[2] = { static_cast<float>(dash->dashWidth), static_cast<float>(dash->dashGap) };

      lv_vector_dsc_set_stroke_dash(dsc.get(), dashes, 2);
    }

    lv_vector_dsc_add_path(dsc.get(), path.get());
    lv_draw_vector(dsc.get());

    // --- handle asymmetric rounded ends (same trick you already use)
    if(ends.has_value() && (ends->start != ends->end))
    {
      const PointF circleCenter = ends->start ? p1 : p2;

      auto capDsc = tVectorDscPtr(lv_vector_dsc_create(&m_layer), &lv_vector_dsc_delete);
      auto capPath = tVectorPathPtr(lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM), &lv_vector_path_delete);

      if(capDsc && capPath)
      {
        lv_fpoint_t center = { static_cast<float>(circleCenter.x), static_cast<float>(circleCenter.y) };

        lv_vector_path_append_circle(capPath.get(), &center, style.width / 2.0f, style.width / 2.0f);

        lv_vector_dsc_set_fill_color(capDsc.get(), lv_color_make(style.color.r, style.color.g, style.color.b));

        lv_vector_dsc_set_fill_opa(capDsc.get(), static_cast<lv_opa_t>(style.color.a * 255.0f));

        lv_vector_dsc_add_path(capDsc.get(), capPath.get());
        lv_draw_vector(capDsc.get());
      }
    }
  }

  void LVGLDrawContext::drawQuadraticBezier(const StrokeStyle style, const Point start, const Point control,
                                            const Point end)
  {
    using tVectorDscPtr = std::unique_ptr<lv_vector_dsc_t, decltype(&lv_vector_dsc_delete)>;
    using tVectorPathPtr = std::unique_ptr<lv_vector_path_t, decltype(&lv_vector_path_delete)>;

    auto dsc = tVectorDscPtr(lv_vector_dsc_create(&m_layer), &lv_vector_dsc_delete);
    auto path = tVectorPathPtr(lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM), &lv_vector_path_delete);

    if(!dsc || !path)
      return;

    lv_fpoint_t p0 = { static_cast<float>(start.x), static_cast<float>(start.y) };
    lv_vector_path_move_to(path.get(), &p0);

    lv_fpoint_t cp = { static_cast<float>(control.x), static_cast<float>(control.y) };

    lv_fpoint_t ep = { static_cast<float>(end.x), static_cast<float>(end.y) };

    lv_vector_path_quad_to(path.get(), &cp, &ep);

    lv_vector_dsc_set_fill_opa(dsc.get(), LV_OPA_TRANSP);

    lv_vector_dsc_set_stroke_color(dsc.get(), lv_color_make(style.color.r, style.color.g, style.color.b));

    lv_vector_dsc_set_stroke_opa(dsc.get(), static_cast<lv_opa_t>(style.color.a * 255.0f));

    lv_vector_dsc_set_stroke_width(dsc.get(), static_cast<float>(style.width));

    lv_vector_dsc_set_stroke_cap(dsc.get(), LV_VECTOR_STROKE_CAP_BUTT);

    lv_vector_dsc_set_stroke_join(dsc.get(), LV_VECTOR_STROKE_JOIN_ROUND);

    lv_vector_dsc_add_path(dsc.get(), path.get());
    lv_draw_vector(dsc.get());
  }

  void LVGLDrawContext::drawQuadraticBezier(const StrokeStyle style, const Point start, const Point control,
                                            const Point end, std::optional<RoundedEnds> ends)
  {

    using tVectorDscPtr = std::unique_ptr<lv_vector_dsc_t, decltype(&lv_vector_dsc_delete)>;
    using tVectorPathPtr = std::unique_ptr<lv_vector_path_t, decltype(&lv_vector_path_delete)>;
    auto dsc = tVectorDscPtr(lv_vector_dsc_create(&m_layer), &lv_vector_dsc_delete);
    auto path = tVectorPathPtr(lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM), &lv_vector_path_delete);

    if(!dsc || !path)
      return;

    lv_fpoint_t p0 = { static_cast<float>(start.x), static_cast<float>(start.y) };
    lv_vector_path_move_to(path.get(), &p0);

    lv_fpoint_t cp = { static_cast<float>(control.x), static_cast<float>(control.y) };

    lv_fpoint_t ep = { static_cast<float>(end.x), static_cast<float>(end.y) };

    lv_vector_path_quad_to(path.get(), &cp, &ep);

    lv_vector_dsc_set_fill_opa(dsc.get(), LV_OPA_TRANSP);

    lv_vector_dsc_set_stroke_color(dsc.get(), lv_color_make(style.color.r, style.color.g, style.color.b));

    lv_vector_dsc_set_stroke_opa(dsc.get(), static_cast<lv_opa_t>(style.color.a * 255.0f));

    lv_vector_dsc_set_stroke_width(dsc.get(), static_cast<float>(style.width));

    lv_vector_dsc_set_stroke_join(dsc.get(), LV_VECTOR_STROKE_JOIN_ROUND);

    lv_vector_stroke_cap_t cap = LV_VECTOR_STROKE_CAP_BUTT;

    if(ends.has_value())
    {
      if(ends->start && ends->end)
      {
        cap = LV_VECTOR_STROKE_CAP_ROUND;
      }
      else if(ends->start || ends->end)
      {
        cap = LV_VECTOR_STROKE_CAP_BUTT;
      }
    }

    lv_vector_dsc_set_stroke_cap(dsc.get(), cap);

    lv_vector_dsc_add_path(dsc.get(), path.get());
    lv_draw_vector(dsc.get());

    if(ends.has_value() && (ends->start != ends->end))
    {
      const Point circleCenter = ends->start ? start : end;

      auto capDsc = tVectorDscPtr(lv_vector_dsc_create(&m_layer), &lv_vector_dsc_delete);
      auto capPath = tVectorPathPtr(lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM), &lv_vector_path_delete);

      if(capDsc && capPath)
      {
        lv_fpoint_t center = { static_cast<float>(circleCenter.x), static_cast<float>(circleCenter.y) };

        lv_vector_path_append_circle(capPath.get(), &center, style.width / 2.0f, style.width / 2.0f);

        lv_vector_dsc_set_fill_color(capDsc.get(), lv_color_make(style.color.r, style.color.g, style.color.b));

        lv_vector_dsc_set_fill_opa(capDsc.get(), static_cast<lv_opa_t>(style.color.a * 255.0f));

        lv_vector_dsc_add_path(capDsc.get(), capPath.get());
        lv_draw_vector(capDsc.get());
      }
    }
  }

  void LVGLDrawContext::strokeRect(const StrokeStyle style, const Rect rect)
  {
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_opa = LV_OPA_TRANSP;
    rect_dsc.border_color = lv_color_make(style.color.r, style.color.g, style.color.b);
    rect_dsc.border_width = style.width;
    rect_dsc.border_opa = static_cast<lv_opa_t>(style.color.a * 255.0);

    lv_area_t area;
    area.x1 = rect.pos.x;
    area.y1 = rect.pos.y;
    area.x2 = rect.pos.x + rect.size.w - 1;
    area.y2 = rect.pos.y + rect.size.h - 1;

    lv_draw_rect(&m_layer, &rect_dsc, &area);
  }

  void LVGLDrawContext::strokeRoundedRect(StrokeStyle style, Rect rect, RoundedCorner rc)
  {
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_opa = LV_OPA_TRANSP;
    rect_dsc.border_color = lv_color_make(style.color.r, style.color.g, style.color.b);
    rect_dsc.border_width = style.width;
    rect_dsc.border_opa = static_cast<lv_opa_t>(style.color.a * 255.0);

    rect_dsc.radius = rc.radius;

    lv_area_t area;
    area.x1 = rect.pos.x;
    area.y1 = rect.pos.y;
    area.x2 = rect.pos.x + rect.size.w - 1;
    area.y2 = rect.pos.y + rect.size.h - 1;

    lv_draw_rect(&m_layer, &rect_dsc, &area);
  }

  using tVectorDscPtr = std::unique_ptr<lv_vector_dsc_t, decltype(&lv_vector_dsc_delete)>;
  using tVectorPathPtr = std::unique_ptr<lv_vector_path_t, decltype(&lv_vector_path_delete)>;

  void LVGLDrawContext::strokeCustomRoundedRect(StrokeStyle style, Rect r, int topLeft, int topRight, int bottomLeft,
                                                int bottomRight)
  {
    auto dsc = tVectorDscPtr(lv_vector_dsc_create(&m_layer), &lv_vector_dsc_delete);
    auto path = tVectorPathPtr(lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM), &lv_vector_path_delete);

    if(!dsc || !path)
      return;

    float x = r.pos.x;
    float y = r.pos.y;
    float w = r.size.w;
    float h = r.size.h;

    float halfWidth = style.width / 2.0f;
    x += halfWidth;
    y += halfWidth;
    w -= style.width;
    h -= style.width;

    lv_fpoint_t p;

    p = { x + topLeft, y };
    lv_vector_path_move_to(path.get(), &p);

    p = { x + w - topRight, y };
    lv_vector_path_line_to(path.get(), &p);
    lv_fpoint_t cp = { x + w, y };
    lv_fpoint_t ep = { x + w, y + topRight };
    lv_vector_path_quad_to(path.get(), &cp, &ep);

    p = { x + w, y + h - bottomRight };
    lv_vector_path_line_to(path.get(), &p);
    cp = { x + w, y + h };
    ep = { x + w - bottomRight, y + h };
    lv_vector_path_quad_to(path.get(), &cp, &ep);

    p = { x + bottomLeft, y + h };
    lv_vector_path_line_to(path.get(), &p);
    cp = { x, y + h };
    ep = { x, y + h - bottomLeft };
    lv_vector_path_quad_to(path.get(), &cp, &ep);

    p = { x, y + topLeft };
    lv_vector_path_line_to(path.get(), &p);
    cp = { x, y };
    ep = { x + topLeft, y };
    lv_vector_path_quad_to(path.get(), &cp, &ep);

    lv_vector_path_close(path.get());

    lv_vector_dsc_set_stroke_color(dsc.get(), lv_color_make(style.color.r, style.color.g, style.color.b));
    lv_vector_dsc_set_stroke_opa(dsc.get(), static_cast<lv_opa_t>(style.color.a * 255.0));
    lv_vector_dsc_set_stroke_width(dsc.get(), style.width);
    lv_vector_dsc_set_stroke_join(dsc.get(), LV_VECTOR_STROKE_JOIN_ROUND);
    lv_vector_dsc_set_stroke_cap(dsc.get(), LV_VECTOR_STROKE_CAP_ROUND);

    lv_vector_dsc_add_path(dsc.get(), path.get());
    lv_draw_vector(dsc.get());
  };

  void LVGLDrawContext::fillRect(const Color color, const Rect rect)
  {
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_color = lv_color_make(color.r, color.g, color.b);
    rect_dsc.bg_opa = static_cast<lv_opa_t>(color.a * 255.0);
    rect_dsc.border_opa = LV_OPA_TRANSP;

    lv_area_t area;
    area.x1 = rect.pos.x;
    area.y1 = rect.pos.y;
    area.x2 = rect.pos.x + rect.size.w - 1;
    area.y2 = rect.pos.y + rect.size.h - 1;

    lv_draw_rect(&m_layer, &rect_dsc, &area);
  }

  void LVGLDrawContext::fillRoundedRect(const Color color, const Rect rect, const RoundedCorner rc)
  {
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_color = lv_color_make(color.r, color.g, color.b);
    rect_dsc.bg_opa = static_cast<lv_opa_t>(color.a * 255.0);
    rect_dsc.border_opa = LV_OPA_TRANSP;
    rect_dsc.radius = rc.radius;

    lv_area_t area;
    area.x1 = rect.pos.x;
    area.y1 = rect.pos.y;
    area.x2 = rect.pos.x + rect.size.w - 1;
    area.y2 = rect.pos.y + rect.size.h - 1;

    lv_draw_rect(&m_layer, &rect_dsc, &area);
  }

  void LVGLDrawContext::fillCustomRoundedRect(Color color, Rect rect, int topLeft, int topRight, int bottomLeft,
                                              int bottomRight)
  {
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_color = lv_color_make(color.r, color.g, color.b);
    rect_dsc.bg_opa = static_cast<lv_opa_t>(color.a * 255.0);
    rect_dsc.border_opa = LV_OPA_TRANSP;

    rect_dsc.radius = std::max({ topLeft, topRight, bottomLeft, bottomRight });

    auto dsc = tVectorDscPtr(lv_vector_dsc_create(&m_layer), &lv_vector_dsc_delete);
    auto path = tVectorPathPtr(lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM), &lv_vector_path_delete);

    if(!dsc || !path)
      return;

    float x = rect.pos.x;
    float y = rect.pos.y;
    float w = rect.size.w;
    float h = rect.size.h;

    lv_fpoint_t p;

    p = { x + topLeft, y };
    lv_vector_path_move_to(path.get(), &p);

    p = { x + w - topRight, y };
    lv_vector_path_line_to(path.get(), &p);
    lv_fpoint_t cp = { x + w, y };
    lv_fpoint_t ep = { x + w, y + topRight };
    lv_vector_path_quad_to(path.get(), &cp, &ep);

    p = { x + w, y + h - bottomRight };
    lv_vector_path_line_to(path.get(), &p);
    cp = { x + w, y + h };
    ep = { x + w - bottomRight, y + h };
    lv_vector_path_quad_to(path.get(), &cp, &ep);

    p = { x + bottomLeft, y + h };
    lv_vector_path_line_to(path.get(), &p);
    cp = { x, y + h };
    ep = { x, y + h - bottomLeft };
    lv_vector_path_quad_to(path.get(), &cp, &ep);

    p = { x, y + topLeft };
    lv_vector_path_line_to(path.get(), &p);
    cp = { x, y };
    ep = { x + topLeft, y };
    lv_vector_path_quad_to(path.get(), &cp, &ep);

    lv_vector_path_close(path.get());

    lv_vector_dsc_set_fill_color(dsc.get(), lv_color_make(color.r, color.g, color.b));
    lv_vector_dsc_set_fill_opa(dsc.get(), static_cast<lv_opa_t>(color.a * 255.0));
    lv_vector_dsc_add_path(dsc.get(), path.get());
    lv_draw_vector(dsc.get());
  }

  void LVGLDrawContext::fillPolygon(StrokeStyle stroke, Color fill, std::vector<Point> points)
  {
    if(points.size() < 3)
      return;

    auto dsc = tVectorDscPtr(lv_vector_dsc_create(&m_layer), &lv_vector_dsc_delete);
    if(!dsc)
      return;

    auto path = tVectorPathPtr(lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM), &lv_vector_path_delete);
    if(!path)
      return;

    lv_fpoint_t first_point = { static_cast<float>(points[0].x), static_cast<float>(points[0].y) };
    lv_vector_path_move_to(path.get(), &first_point);

    for(size_t i = 1; i < points.size(); ++i)
    {
      lv_fpoint_t point = { static_cast<float>(points[i].x), static_cast<float>(points[i].y) };
      lv_vector_path_line_to(path.get(), &point);
    }

    lv_vector_path_close(path.get());

    lv_vector_dsc_set_fill_color(dsc.get(), lv_color_make(fill.r, fill.g, fill.b));
    lv_vector_dsc_set_fill_opa(dsc.get(), static_cast<lv_opa_t>(fill.a * 255.0));
    lv_vector_dsc_set_stroke_color(dsc.get(), lv_color_make(stroke.color.r, stroke.color.g, stroke.color.b));
    lv_vector_dsc_set_stroke_opa(dsc.get(), static_cast<lv_opa_t>(stroke.color.a * 255.0));
    lv_vector_dsc_set_stroke_width(dsc.get(), static_cast<float>(stroke.width));
    lv_vector_dsc_add_path(dsc.get(), path.get());

    lv_draw_vector(dsc.get());
  }

  void LVGLDrawContext::fillRoundedPolygon(StrokeStyle stroke, Color fill, std::vector<Point> points, RoundedCorner rc)
  {
    if(points.size() < 3)
      return;

    if(rc.radius <= 0)
    {
      fillPolygon(stroke, fill, points);
      return;
    }

    auto dsc = tVectorDscPtr(lv_vector_dsc_create(&m_layer), &lv_vector_dsc_delete);
    auto path = tVectorPathPtr(lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM), &lv_vector_path_delete);
    if(!dsc || !path)
      return;

    size_t count = points.size();
    float radius = static_cast<float>(rc.radius);

    for(size_t i = 0; i < count; ++i)
    {
      Point p1 = points[(i + count - 1) % count];
      Point p2 = points[i];
      Point p3 = points[(i + 1) % count];

      float dx1 = static_cast<float>(p1.x - p2.x);
      float dy1 = static_cast<float>(p1.y - p2.y);
      float dx2 = static_cast<float>(p3.x - p2.x);
      float dy2 = static_cast<float>(p3.y - p2.y);

      float len1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
      float len2 = std::sqrt(dx2 * dx2 + dy2 * dy2);

      float r = std::min({ radius, len1 / 2.0f, len2 / 2.0f });

      lv_fpoint_t start_arc = { p2.x + (dx1 / len1) * r, p2.y + (dy1 / len1) * r };
      lv_fpoint_t end_arc = { p2.x + (dx2 / len2) * r, p2.y + (dy2 / len2) * r };
      lv_fpoint_t control_point = { static_cast<float>(p2.x), static_cast<float>(p2.y) };

      if(i == 0)
        lv_vector_path_move_to(path.get(), &start_arc);
      else
        lv_vector_path_line_to(path.get(), &start_arc);

      lv_vector_path_quad_to(path.get(), &control_point, &end_arc);
    }

    lv_vector_path_close(path.get());

    lv_vector_dsc_set_fill_color(dsc.get(), lv_color_make(fill.r, fill.g, fill.b));
    lv_vector_dsc_set_fill_opa(dsc.get(), static_cast<lv_opa_t>(fill.a * 255.0));
    lv_vector_dsc_set_stroke_color(dsc.get(), lv_color_make(stroke.color.r, stroke.color.g, stroke.color.b));
    lv_vector_dsc_set_stroke_opa(dsc.get(), static_cast<lv_opa_t>(stroke.color.a * 255.0));
    lv_vector_dsc_set_stroke_width(dsc.get(), static_cast<float>(stroke.width));

    lv_vector_dsc_set_stroke_join(dsc.get(), LV_VECTOR_STROKE_JOIN_ROUND);

    lv_vector_dsc_add_path(dsc.get(), path.get());
    lv_draw_vector(dsc.get());
  }

  void LVGLDrawContext::fillArc(const ArcDrawOptions &arcOptions)
  {
    const auto dsc = tVectorDscPtr(lv_vector_dsc_create(&m_layer), &lv_vector_dsc_delete);
    if(!dsc)
      return;

    const auto path = tVectorPathPtr(lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM), &lv_vector_path_delete);
    if(!path)
      return;

    const lv_fpoint_t center = { static_cast<float>(arcOptions.position.x), static_cast<float>(arcOptions.position.y) };

    lv_vector_path_append_arc(path.get(), &center, arcOptions.radius, arcOptions.startAngle, arcOptions.sweepAngle,
                              false);

    lv_vector_dsc_set_fill_opa(dsc.get(), LV_OPA_0);

    const auto &color = arcOptions.color;

    lv_vector_dsc_set_stroke_color(dsc.get(), lv_color_make(color.r, color.g, color.b));
    lv_vector_dsc_set_stroke_opa(dsc.get(), static_cast<lv_opa_t>(color.a * static_cast<float>(LV_OPA_COVER)));
    lv_vector_dsc_set_stroke_width(dsc.get(), static_cast<float>(arcOptions.strokeWidth));

    if(arcOptions.dashes.has_value())
    {
      auto dashes = arcOptions.dashes.value();
      lv_vector_dsc_set_stroke_dash(dsc.get(), dashes.data(), dashes.size());
    }

    lv_vector_dsc_add_path(dsc.get(), path.get());

    lv_draw_vector(dsc.get());
  }

  void LVGLDrawContext::drawSegmentedArc(const SegmentedArcDrawOptions &props)
  {
    const auto segmentAngle = props.sweep / static_cast<float>(props.numSegments);
    const auto numLines = props.numSegments + 1;
    const auto lineAngle = static_cast<float>(props.dashWidth) * segmentAngle / 100.0f;

    if(props.lineColor.a > 0)
    {
      for(int i = 0; i < numLines; i++)
      {
        const auto lineAnglePos = props.startAngle + i * segmentAngle;

        const ArcDrawOptions lineArc = { .position = props.center,
                                         .color = props.lineColor,
                                         .radius = props.radius,
                                         .strokeWidth = props.strokeWidth,
                                         .startAngle = lineAnglePos,
                                         .sweepAngle = lineAngle };

        fillArc(lineArc);
      }
    }

    if(props.spaceColor.a > 0)
    {
      for(int i = 0; i < props.numSegments; i++)
      {
        const auto spaceStartAngle = props.startAngle + i * segmentAngle + lineAngle;
        const auto spaceSweepAngle = segmentAngle - lineAngle;

        const ArcDrawOptions spaceArc = { .position = props.center,
                                          .color = props.spaceColor,
                                          .radius = props.radius,
                                          .strokeWidth = props.strokeWidth,
                                          .startAngle = spaceStartAngle,
                                          .sweepAngle = spaceSweepAngle };

        fillArc(spaceArc);
      }
    }
  }

  void LVGLDrawContext::drawText(Text t, Font f, Rect r, Color c, TextAlign ta, VerticalAlign va)
  {
    const auto &font = s_fontStorage->getFont(f);
    const auto textWidth = font.getStringWidth(t.text);
    const auto startX = LabelShared::computeStartX(r.size.w, textWidth, ta);
    const auto startY = LabelShared::computeStartYSingle(r.size.h, font, t.text, va);
    drawText(t.text, r.pos.x + startX, r.pos.y + startY, font, c);
  }

  void LVGLDrawContext::putBitmap(const Bitmap &image, Point p, std::optional<Color> colorOverride)
  {
    if(!image.start || image.width <= 0 || image.height <= 0)
      return;

    const auto *canvas = reinterpret_cast<lv_canvas_t *>(&m_canvas);
    const auto *draw_buf = canvas->draw_buf;

    if(!draw_buf || draw_buf->header.cf != LV_COLOR_FORMAT_ARGB8888)
      return;

    const auto canvas_width = draw_buf->header.w;
    const auto canvas_height = draw_buf->header.h;
    const auto canvas_stride = draw_buf->header.stride;

    const int src_width = image.width;
    const int src_height = image.height;
    const int src_stride = image.stride;

    const int start_x = p.x;
    const int start_y = p.y;

    if(start_x >= canvas_width || start_y >= canvas_height)
      return;

    const auto end_x = std::min<int>(start_x + src_width, static_cast<int>(canvas_width));
    const auto end_y = std::min<int>(start_y + src_height, static_cast<int>(canvas_height));
    const auto copy_width = end_x - start_x;
    const auto copy_height = end_y - start_y;

    if(copy_width <= 0 || copy_height <= 0)
      return;

    const auto canvas_data = static_cast<uint8_t *>(draw_buf->data);
    const uint8_t *src_data = image.start;

    for(int y = 0; y < copy_height; ++y)
    {
      const int canvas_y = start_y + y;
      const int src_y = y;

      uint8_t *canvas_row = canvas_data + canvas_y * canvas_stride + start_x * 4;
      const uint8_t *src_row = src_data + src_y * src_stride;

      for(int x = 0; x < copy_width; ++x)
      {
        const int src_offset = x * 4;
        const uint8_t src_a = src_row[src_offset + 3];

        if(src_a > 0)
        {
          uint8_t final_r, final_g, final_b, final_a;

          if(colorOverride.has_value())
          {
            const auto &override_color = colorOverride.value();
            final_r = static_cast<uint8_t>(override_color.r);
            final_g = static_cast<uint8_t>(override_color.g);
            final_b = static_cast<uint8_t>(override_color.b);
            final_a = static_cast<uint8_t>(src_a * override_color.a);
          }
          else
          {
            final_r = src_row[src_offset + 2];
            final_g = src_row[src_offset + 1];
            final_b = src_row[src_offset + 0];
            final_a = src_a;
          }

          const int canvas_offset = x * 4;
          uint8_t *canvas_pixel = canvas_row + canvas_offset;

          if(final_a == 255)
          {
            canvas_pixel[0] = final_b;
            canvas_pixel[1] = final_g;
            canvas_pixel[2] = final_r;
            canvas_pixel[3] = final_a;
          }
          else
          {
            const uint8_t canvas_a = canvas_pixel[3];
            const uint8_t canvas_r = canvas_pixel[2];
            const uint8_t canvas_g = canvas_pixel[1];
            const uint8_t canvas_b = canvas_pixel[0];

            const uint16_t alpha = final_a;
            const uint16_t inv_alpha = 255 - final_a;

            canvas_pixel[0] = (final_b * alpha + canvas_b * inv_alpha) / 255;
            canvas_pixel[1] = (final_g * alpha + canvas_g * inv_alpha) / 255;
            canvas_pixel[2] = (final_r * alpha + canvas_r * inv_alpha) / 255;
            canvas_pixel[3] = std::max(final_a, canvas_a);
          }
        }
      }
    }

    lv_obj_invalidate(&m_canvas);
  }

  void LVGLDrawContext::drawText(const Glib::ustring &text, int x, int y, const FreeTypeFont &font, Color c)
  {
    if(text.empty())
      return;

    const auto *canvas = reinterpret_cast<lv_canvas_t *>(&m_canvas);
    const auto *draw_buf = canvas->draw_buf;
    if(!draw_buf)
      return;

    flushLayer();
    font.draw(text, x, y,
              [&](int px, int py, unsigned char coverage) { drawFontPixel(*draw_buf, c, px, py, coverage); });
  }

  void LVGLDrawContext::drawFontPixel(const lv_draw_buf_t &draw_buf, const Color &baseColor, int px, int py,
                                      unsigned char coverage)
  {
    const auto canvas_width = draw_buf.header.w;
    const auto canvas_height = draw_buf.header.h;
    const auto canvas_stride = draw_buf.header.stride;
    auto *canvas_data = draw_buf.data;

    if(px < 0 || py < 0 || px >= canvas_width || py >= canvas_height)
      return;

    uint8_t *canvas_pixel = canvas_data + py * canvas_stride + px * 4;
    const uint8_t src_r = baseColor.r;
    const uint8_t src_g = baseColor.g;
    const uint8_t src_b = baseColor.b;
    const uint8_t src_a = static_cast<uint8_t>(static_cast<float>(coverage) * baseColor.a);

    if(src_a == 0)
      return;

    const uint8_t dst_b = canvas_pixel[0];
    const uint8_t dst_g = canvas_pixel[1];
    const uint8_t dst_r = canvas_pixel[2];
    const uint8_t dst_a = canvas_pixel[3];

    const uint16_t src_a_u16 = src_a;
    const uint16_t dst_a_u16 = dst_a;
    const uint16_t inv_src_a = 255 - src_a_u16;

    const uint16_t out_a_u16 = src_a_u16 + (dst_a_u16 * inv_src_a + 127) / 255;
    if(out_a_u16 == 0)
      return;

    const uint32_t out_a_x255 = static_cast<uint32_t>(out_a_u16) * 255;
    const uint32_t src_factor = static_cast<uint32_t>(src_a_u16) * 255;
    const uint32_t dst_factor = static_cast<uint32_t>(dst_a_u16) * inv_src_a;

    canvas_pixel[2] = static_cast<uint8_t>((src_r * src_factor + dst_r * dst_factor + out_a_x255 / 2) / out_a_x255);
    canvas_pixel[1] = static_cast<uint8_t>((src_g * src_factor + dst_g * dst_factor + out_a_x255 / 2) / out_a_x255);
    canvas_pixel[0] = static_cast<uint8_t>((src_b * src_factor + dst_b * dst_factor + out_a_x255 / 2) / out_a_x255);
    canvas_pixel[3] = static_cast<uint8_t>(out_a_u16);
  }

  void LVGLDrawContext::flushLayer()
  {
    lv_canvas_finish_layer(&m_canvas, &m_layer);
    lv_canvas_init_layer(&m_canvas, &m_layer);
  }

  void LVGLDrawContext::fillEnvelopeArea(Color color, Point start, Point attackCtrl, Point attackEnd, Point decay1End,
                                         Point decay2Ctrl, Point decay2End, Point sustainEnd, Point releaseCtrl,
                                         Point releaseEnd, int bottomY)
  {
    using tVectorDscPtr = std::unique_ptr<lv_vector_dsc_t, decltype(&lv_vector_dsc_delete)>;
    using tVectorPathPtr = std::unique_ptr<lv_vector_path_t, decltype(&lv_vector_path_delete)>;

    auto dsc = tVectorDscPtr(lv_vector_dsc_create(&m_layer), &lv_vector_dsc_delete);
    auto path = tVectorPathPtr(lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM), &lv_vector_path_delete);

    if(!dsc || !path)
      return;

    // Start at bottom-left
    lv_fpoint_t p = { (float) start.x, (float) bottomY };
    lv_vector_path_move_to(path.get(), &p);

    // Go up to start of envelope
    p = { (float) start.x, (float) start.y };
    lv_vector_path_line_to(path.get(), &p);

    // Attack (quad)
    lv_fpoint_t cp = { (float) attackCtrl.x, (float) attackCtrl.y };
    lv_fpoint_t ep = { (float) attackEnd.x, (float) attackEnd.y };
    lv_vector_path_quad_to(path.get(), &cp, &ep);

    // Decay1 (line)
    p = { (float) decay1End.x, (float) decay1End.y };
    lv_vector_path_line_to(path.get(), &p);

    // Decay2 (quad)
    cp = { (float) decay2Ctrl.x, (float) decay2Ctrl.y };
    ep = { (float) decay2End.x, (float) decay2End.y };
    lv_vector_path_quad_to(path.get(), &cp, &ep);

    // Sustain (line)
    p = { (float) sustainEnd.x, (float) sustainEnd.y };
    lv_vector_path_line_to(path.get(), &p);

    // Release (quad)
    cp = { (float) releaseCtrl.x, (float) releaseCtrl.y };
    ep = { (float) releaseEnd.x, (float) releaseEnd.y };
    lv_vector_path_quad_to(path.get(), &cp, &ep);

    // Go down to baseline
    p = { (float) releaseEnd.x, (float) bottomY };
    lv_vector_path_line_to(path.get(), &p);

    // Close back to start
    lv_vector_path_close(path.get());

    // Fill
    lv_vector_dsc_set_fill_color(dsc.get(), lv_color_make(color.r, color.g, color.b));

    lv_vector_dsc_set_fill_opa(dsc.get(), static_cast<lv_opa_t>(color.a * 255.0f));

    // No stroke
    lv_vector_dsc_set_stroke_opa(dsc.get(), LV_OPA_TRANSP);

    lv_vector_dsc_add_path(dsc.get(), path.get());
    lv_draw_vector(dsc.get());
  }

}
