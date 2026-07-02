#include <compose/widgets/DrawContext.h>
#include <compose/SVGDocumentCache.h>
#include <compose/SVGRender.h>
#include "src/draw/lv_draw.h"
#include "src/misc/lv_color.h"
#include "src/misc/lv_area.h"
#include <src/widgets/canvas/lv_canvas.h>
#include "src/draw/lv_draw_vector.h"
#include <memory>
#include <type_traits>
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include "compose/widgets/Label.h"
#include <compose/FreeTypeFont.h>
#include "src/widgets/canvas/lv_canvas_private.h"
#include "compose/widgets/LabelShared.h"
#include <cassert>

namespace Compose
{
  std::unique_ptr<FontStorage> s_fontStorage = nullptr;
  using tVectorDsc = std::remove_pointer_t<decltype(lv_vector_dsc_create(static_cast<lv_layer_t *>(nullptr)))>;
  using tVectorDscPtr = std::unique_ptr<tVectorDsc, decltype(&lv_vector_dsc_delete)>;
  using tVectorPathPtr = std::unique_ptr<lv_vector_path_t, decltype(&lv_vector_path_delete)>;

  namespace
  {
    Point translate(Point point, Point offset)
    {
      point.x += offset.x;
      point.y += offset.y;
      return point;
    }

    Rect translate(Rect rect, Point offset)
    {
      rect.pos = translate(rect.pos, offset);
      return rect;
    }

    std::vector<Point> translate(std::vector<Point> points, Point offset)
    {
      for(auto &point : points)
        point = translate(point, offset);

      return points;
    }

    DrawContext::ArcDrawOptions translate(DrawContext::ArcDrawOptions arc, Point offset)
    {
      arc.position = translate(arc.position, offset);
      return arc;
    }

    struct ClippedLineSegment
    {
      Point start;
      Point end;
      bool startWasClipped {};
      bool endWasClipped {};
    };

    bool clipLineBoundary(double p, double q, double &start, double &end)
    {
      bool accepted = true;

      if(p == 0.0)
      {
        if(q < 0.0)
          accepted = false;
      }
      else
      {
        const auto intersection = q / p;

        if(p < 0.0)
        {
          if(intersection > end)
            accepted = false;
          else if(intersection > start)
            start = intersection;
        }
        else if(intersection < start)
        {
          accepted = false;
        }
        else if(intersection < end)
        {
          end = intersection;
        }
      }

      return accepted;
    }

    std::optional<ClippedLineSegment> clipLineToArea(const Point start, const Point end, const lv_area_t &clipArea)
    {
      const auto dx = static_cast<double>(end.x - start.x);
      const auto dy = static_cast<double>(end.y - start.y);
      double clippedStart = 0.0;
      double clippedEnd = 1.0;

      const auto accepted = clipLineBoundary(-dx, start.x - clipArea.x1, clippedStart, clippedEnd) && clipLineBoundary(dx, clipArea.x2 - start.x, clippedStart, clippedEnd)
          && clipLineBoundary(-dy, start.y - clipArea.y1, clippedStart, clippedEnd) && clipLineBoundary(dy, clipArea.y2 - start.y, clippedStart, clippedEnd);

      std::optional<ClippedLineSegment> ret;

      if(accepted)
      {
        const auto toPoint = [&](double t) {
          return Point { .x = std::clamp(static_cast<int>(std::lround(start.x + dx * t)), clipArea.x1, clipArea.x2),
                         .y = std::clamp(static_cast<int>(std::lround(start.y + dy * t)), clipArea.y1, clipArea.y2) };
        };

        ret = ClippedLineSegment { .start = toPoint(clippedStart), .end = toPoint(clippedEnd), .startWasClipped = clippedStart > 0.0, .endWasClipped = clippedEnd < 1.0 };
      }

      return ret;
    }

    lv_area_t toArea(const Rect &rect)
    {
      return { .x1 = rect.pos.x, .y1 = rect.pos.y, .x2 = rect.pos.x + rect.size.w - 1, .y2 = rect.pos.y + rect.size.h - 1 };
    }

    std::optional<lv_area_t> clipAreaToPane(const lv_area_t &area, lv_area_t pane, int padding = 0)
    {
      if(padding > 0)
        lv_area_increase(&pane, padding, padding);

      lv_area_t ret { .x1 = std::max(area.x1, pane.x1), .y1 = std::max(area.y1, pane.y1), .x2 = std::min(area.x2, pane.x2), .y2 = std::min(area.y2, pane.y2) };

      return ret.x1 <= ret.x2 && ret.y1 <= ret.y2 ? std::make_optional(ret) : std::nullopt;
    }

    std::optional<lv_area_t> getBoundingArea(const std::vector<Point> &points, int padding = 0)
    {
      std::optional<lv_area_t> ret;

      if(!points.empty())
      {
        lv_area_t area { .x1 = points.front().x, .y1 = points.front().y, .x2 = points.front().x, .y2 = points.front().y };

        for(const auto &point : points)
        {
          area.x1 = std::min(area.x1, point.x);
          area.y1 = std::min(area.y1, point.y);
          area.x2 = std::max(area.x2, point.x);
          area.y2 = std::max(area.y2, point.y);
        }

        if(padding > 0)
          lv_area_increase(&area, padding, padding);

        ret = area;
      }

      return ret;
    }

    DrawContext::QuadPathSegment translate(DrawContext::QuadPathSegment segment, Point offset)
    {
      segment.a = translate(segment.a, offset);
      segment.b = translate(segment.b, offset);
      return segment;
    }

    std::vector<DrawContext::tPathSegment> translate(std::vector<DrawContext::tPathSegment> segments, Point offset)
    {
      for(auto &segment : segments)
        std::visit([&](auto &v) { v = translate(v, offset); }, segment);

      return segments;
    }

    void collectPoints(std::vector<Point> &points, Point p)
    {
      points.push_back(p);
    }

    void collectPoints(std::vector<Point> &points, const DrawContext::QuadPathSegment &segment)
    {
      points.push_back(segment.a);
      points.push_back(segment.b);
    }

    std::optional<lv_area_t> getBoundingArea(const std::vector<DrawContext::tPathSegment> &segments, int padding = 0)
    {
      std::vector<Point> points;
      for(const auto &segment : segments)
        std::visit([&](const auto &v) { collectPoints(points, v); }, segment);

      return getBoundingArea(points, padding);
    }
  }

  DrawContext::ScopedOffset::ScopedOffset(DrawContext &context, Point offset)
      : m_context(&context)
  {
    m_context->pushOffset(offset);
  }

  DrawContext::ScopedOffset::~ScopedOffset()
  {
    if(m_context)
      m_context->popOffset();
  }

  DrawContext::ScopedOffset::ScopedOffset(ScopedOffset &&other) noexcept
      : m_context(other.m_context)
  {
    other.m_context = nullptr;
  }

  DrawContext::ScopedOffset DrawContext::offset(Point offset)
  {
    return ScopedOffset(*this, offset);
  }

  std::vector<DrawContext::tPathSegment> DrawContext::toPathSegments(const std::vector<Point> &arr)
  {
    std::vector<tPathSegment> ret;
    for(auto p : arr)
      ret.emplace_back(p);
    return ret;
  }

  void DrawContext::fillPolygon(StrokeStyle stroke, Color fill, const std::vector<Point> &segments)
  {
    fillPolygon(stroke, fill, toPathSegments(segments));
  }

  LVGLDrawContext::LVGLDrawContext(tCanvas &ctx)
      : m_layer {}
      , m_canvas(ctx)
  {
    lv_canvas_init_layer(&m_canvas, &m_layer);
    lv_canvas_fill_bg(&m_canvas, lv_color_black(), LV_OPA_0);
  }

  LVGLDrawContext::~LVGLDrawContext()
  {
    lv_canvas_finish_layer(&m_canvas, &m_layer);
  }

  void LVGLDrawContext::flushLayer()
  {
    lv_canvas_finish_layer(&m_canvas, &m_layer);
    lv_canvas_init_layer(&m_canvas, &m_layer);
  }

  void LVGLDrawContext::pushOffset(Point offset)
  {
    m_offsetStack.push_back(m_currentOffset);
    m_currentOffset = translate(m_currentOffset, offset);
  }

  void LVGLDrawContext::popOffset()
  {
    assert(!m_offsetStack.empty());
    m_currentOffset = m_offsetStack.back();
    m_offsetStack.pop_back();
  }

  void LVGLDrawContext::drawLine(const StrokeStyle style, const Point p1, const Point p2)
  {
    drawLine(style, p1, p2, std::nullopt, std::nullopt);
  }

  void LVGLDrawContext::drawLine(StrokeStyle style, Point p1, Point p2, std::optional<LineDashOptions> dash, std::optional<RoundedEnds> ends)
  {
    p1 = translate(p1, m_currentOffset);
    p2 = translate(p2, m_currentOffset);

    if(auto clippedLine = clipLineToArea(p1, p2, m_layer._clip_area))
    {
      p1 = clippedLine->start;
      p2 = clippedLine->end;
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
        line_dsc.round_end = ends.value().end && !clippedLine->endWasClipped;
        line_dsc.round_start = ends.value().start && !clippedLine->startWasClipped;
      }

      lv_draw_line(&m_layer, &line_dsc);
    }
  }

  void LVGLDrawContext::drawPath(StrokeStyle style, const std::vector<Point> &points, std::optional<LineDashOptions> dash, std::optional<RoundedEnds> ends)
  {
    if(points.size() < 2)
      return;

    const auto translatedPoints = translate(points, m_currentOffset);

    if(auto bounds = getBoundingArea(translatedPoints, style.width / 2))
    {
      if(!clipAreaToPane(*bounds, m_layer._clip_area))
        return;
    }

    auto dsc = tVectorDscPtr(lv_vector_dsc_create(&m_layer), &lv_vector_dsc_delete);
    auto path = tVectorPathPtr(lv_vector_path_create(LV_VECTOR_PATH_QUALITY_HIGH), &lv_vector_path_delete);
    if(!dsc || !path)
      return;

    lv_fpoint_t first { static_cast<float>(translatedPoints[0].x), static_cast<float>(translatedPoints[0].y) };
    lv_vector_path_move_to(path.get(), &first);

    for(size_t i = 1; i < translatedPoints.size(); i++)
    {
      const lv_fpoint_t point { static_cast<float>(translatedPoints[i].x), static_cast<float>(translatedPoints[i].y) };
      lv_vector_path_line_to(path.get(), &point);
    }

    lv_vector_dsc_set_fill_opa(dsc.get(), LV_OPA_0);
    lv_vector_dsc_set_stroke_color(dsc.get(), lv_color_make(style.color.r, style.color.g, style.color.b));
    lv_vector_dsc_set_stroke_opa(dsc.get(), static_cast<lv_opa_t>(style.color.a * 255.0));
    float strokeWidth = static_cast<float>(style.width);

    lv_vector_dsc_set_stroke_width(dsc.get(), strokeWidth);
    lv_vector_dsc_set_stroke_join(dsc.get(), LV_VECTOR_STROKE_JOIN_ROUND);

    if(ends.has_value())
    {
      if(ends.value().start && ends.value().end)
        lv_vector_dsc_set_stroke_cap(dsc.get(), LV_VECTOR_STROKE_CAP_ROUND);
      else
        lv_vector_dsc_set_stroke_cap(dsc.get(), LV_VECTOR_STROKE_CAP_BUTT);
    }

    if(dash.has_value())
    {
      const auto d = dash.value();
      std::array<float, 2> dashPattern { static_cast<float>(d.dashWidth), static_cast<float>(d.dashGap) };
      lv_vector_dsc_set_stroke_dash(dsc.get(), dashPattern.data(), dashPattern.size());
    }

    lv_vector_dsc_add_path(dsc.get(), path.get());
    lv_draw_vector(dsc.get());
  }

  void LVGLDrawContext::drawLines(StrokeStyle style, const std::vector<Point> &points)
  {
    std::optional<Point> prevPoint;
    for(auto &p : points)
    {
      if(prevPoint)
        drawLine(style, prevPoint.value(), p);
      prevPoint = p;
    }
  }

  void LVGLDrawContext::drawVectorLine(StrokeStyle style, const std::vector<PointF> &points,
                                       std::optional<LineDashOptions> dash, std::optional<RoundedEnds> ends)
  {

    if(const auto dsc = tVectorDscPtr(lv_vector_dsc_create(&m_layer), &lv_vector_dsc_delete))
    {
      if(const auto path = tVectorPathPtr(lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM), &lv_vector_path_delete))
      {
        if(points.size() >= 2)
        {
          auto fPoints = std::vector<lv_fpoint_t> { };
          for(const auto &p : points)
          {
            fPoints.emplace_back(lv_fpoint_t { p.x + static_cast<float>(m_currentOffset.x),
                                               p.y + static_cast<float>(m_currentOffset.y) });
          }
          lv_vector_path_move_to(path.get(), &fPoints.front());
          for(size_t i = 1; i < fPoints.size(); i++)
          {
            lv_vector_path_line_to(path.get(), &fPoints[i]);
          }
          lv_vector_dsc_set_fill_opa(dsc.get(), LV_OPA_TRANSP);
          lv_vector_dsc_set_stroke_color(dsc.get(), lv_color_make(style.color.r, style.color.g, style.color.b));
          lv_vector_dsc_set_stroke_opa(dsc.get(), static_cast<lv_opa_t>(style.color.a * 255.0f));
          lv_vector_dsc_set_stroke_width(dsc.get(), static_cast<float>(style.width));
          lv_vector_dsc_set_stroke_join(dsc.get(), LV_VECTOR_STROKE_JOIN_ROUND);
          lv_vector_stroke_cap_t cap = LV_VECTOR_STROKE_CAP_BUTT;

          if(ends.has_value())
          {
            if(ends->start && ends->end)
              cap = LV_VECTOR_STROKE_CAP_ROUND;
            else
              cap = LV_VECTOR_STROKE_CAP_BUTT;
          }

          lv_vector_dsc_set_stroke_cap(dsc.get(), cap);

          if(dash.has_value())
          {
            float dashes[2] = { static_cast<float>(dash->dashWidth), static_cast<float>(dash->dashGap) };
            lv_vector_dsc_set_stroke_dash(dsc.get(), dashes, 2);
          }

          lv_vector_dsc_add_path(dsc.get(), path.get());
          lv_draw_vector(dsc.get());

          if(ends.has_value() && (ends->start != ends->end))
          {
            auto circleCenter = ends->start ? points.front() : points.back();
            circleCenter.x += static_cast<float>(m_currentOffset.x);
            circleCenter.y += static_cast<float>(m_currentOffset.y);

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
      }
    }
  }

  void LVGLDrawContext::drawVectorLine(StrokeStyle style, PointF p1, PointF p2, std::optional<LineDashOptions> dash,
                                       std::optional<RoundedEnds> ends)
  {
    p1.x += static_cast<float>(m_currentOffset.x);
    p1.y += static_cast<float>(m_currentOffset.y);
    p2.x += static_cast<float>(m_currentOffset.x);
    p2.y += static_cast<float>(m_currentOffset.y);


    if(auto dsc = tVectorDscPtr(lv_vector_dsc_create(&m_layer), &lv_vector_dsc_delete))
    {
      if(auto path = tVectorPathPtr(lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM), &lv_vector_path_delete))
      {
        lv_fpoint_t p0 = { (p1.x), (p1.y) };
        lv_fpoint_t p1f = { (p2.x), (p2.y) };

        lv_vector_path_move_to(path.get(), &p0);
        lv_vector_path_line_to(path.get(), &p1f);

        lv_vector_dsc_set_fill_opa(dsc.get(), LV_OPA_TRANSP);
        lv_vector_dsc_set_stroke_color(dsc.get(), lv_color_make(style.color.r, style.color.g, style.color.b));
        lv_vector_dsc_set_stroke_opa(dsc.get(), static_cast<lv_opa_t>(style.color.a * 255.0f));
        lv_vector_dsc_set_stroke_width(dsc.get(), static_cast<float>(style.width));
        lv_vector_dsc_set_stroke_join(dsc.get(), LV_VECTOR_STROKE_JOIN_ROUND);
        lv_vector_stroke_cap_t cap = LV_VECTOR_STROKE_CAP_BUTT;

        if(ends.has_value())
        {
          if(ends->start && ends->end)
            cap = LV_VECTOR_STROKE_CAP_ROUND;
          else
            cap = LV_VECTOR_STROKE_CAP_BUTT;
        }

        lv_vector_dsc_set_stroke_cap(dsc.get(), cap);

        if(dash.has_value())
        {
          float dashes[2] = { static_cast<float>(dash->dashWidth), static_cast<float>(dash->dashGap) };
          lv_vector_dsc_set_stroke_dash(dsc.get(), dashes, 2);
        }

        lv_vector_dsc_add_path(dsc.get(), path.get());
        lv_draw_vector(dsc.get());

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
    }
  }


  void LVGLDrawContext::drawQuadraticBezier(const StrokeStyle style, const Point rawStart, const Point rawControl,
                                            const Point rawEnd)
  {
    const Point start = translate(rawStart, m_currentOffset);
    const Point control = translate(rawControl, m_currentOffset);
    const Point end = translate(rawEnd, m_currentOffset);

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
    const auto area = toArea(translate(rect, m_currentOffset));
    if(auto clippedArea = clipAreaToPane(area, m_layer._clip_area, style.width))
    {
      lv_draw_rect_dsc_t rect_dsc;
      lv_draw_rect_dsc_init(&rect_dsc);
      rect_dsc.bg_opa = LV_OPA_TRANSP;
      rect_dsc.border_color = lv_color_make(style.color.r, style.color.g, style.color.b);
      rect_dsc.border_width = style.width;
      rect_dsc.border_opa = static_cast<lv_opa_t>(style.color.a * 255.0);

      lv_draw_rect(&m_layer, &rect_dsc, &*clippedArea);
    }
  }

  void LVGLDrawContext::strokeRoundedRect(StrokeStyle style, Rect rect, RoundedCorner rc)
  {
    rect = translate(rect, m_currentOffset);
    const auto area = toArea(rect);
    if(auto clippedArea = clipAreaToPane(area, m_layer._clip_area, style.width))
    {
      lv_draw_rect_dsc_t rect_dsc;
      lv_draw_rect_dsc_init(&rect_dsc);
      rect_dsc.bg_opa = LV_OPA_TRANSP;
      rect_dsc.border_color = lv_color_make(style.color.r, style.color.g, style.color.b);
      rect_dsc.border_width = style.width;
      rect_dsc.border_opa = static_cast<lv_opa_t>(style.color.a * 255.0);

      rect_dsc.radius = rc.radius;

      lv_draw_rect(&m_layer, &rect_dsc, &*clippedArea);
    }
  }

  void LVGLDrawContext::strokeCustomRoundedRect(StrokeStyle style, Rect r, int topLeft, int topRight, int bottomLeft, int bottomRight)
  {
    r = translate(r, m_currentOffset);

    if(!clipAreaToPane(toArea(r), m_layer._clip_area, style.width))
      return;

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
    const auto area = toArea(translate(rect, m_currentOffset));
    if(auto clippedArea = clipAreaToPane(area, m_layer._clip_area))
    {
      lv_draw_rect_dsc_t rect_dsc;
      lv_draw_rect_dsc_init(&rect_dsc);
      rect_dsc.bg_color = lv_color_make(color.r, color.g, color.b);
      rect_dsc.bg_opa = static_cast<lv_opa_t>(color.a * 255.0);
      rect_dsc.border_opa = LV_OPA_TRANSP;

      lv_draw_rect(&m_layer, &rect_dsc, &*clippedArea);
    }
  }

  void LVGLDrawContext::fillRoundedRect(const Color color, const Rect rect, const RoundedCorner rc)
  {
    const auto area = toArea(translate(rect, m_currentOffset));
    if(auto clippedArea = clipAreaToPane(area, m_layer._clip_area))
    {
      lv_draw_rect_dsc_t rect_dsc;
      lv_draw_rect_dsc_init(&rect_dsc);
      rect_dsc.bg_color = lv_color_make(color.r, color.g, color.b);
      rect_dsc.bg_opa = static_cast<lv_opa_t>(color.a * 255.0);
      rect_dsc.border_opa = LV_OPA_TRANSP;
      rect_dsc.radius = rc.radius;

      lv_draw_rect(&m_layer, &rect_dsc, &*clippedArea);
    }
  }

  void LVGLDrawContext::fillCustomRoundedRect(Color color, Rect rect, int topLeft, int topRight, int bottomLeft, int bottomRight)
  {
    rect = translate(rect, m_currentOffset);

    if(!clipAreaToPane(toArea(rect), m_layer._clip_area))
      return;

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

  void LVGLDrawContext::fillPolygon(StrokeStyle stroke, Color fill, std::vector<tPathSegment> points)
  {
    if(points.size() < 3)
      return;

    points = translate(std::move(points), m_currentOffset);

    if(auto bounds = getBoundingArea(points, stroke.width / 2))
    {
      if(!clipAreaToPane(*bounds, m_layer._clip_area))
        return;
    }

    const auto dsc = tVectorDscPtr(lv_vector_dsc_create(&m_layer), &lv_vector_dsc_delete);
    const auto path = tVectorPathPtr(lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM), &lv_vector_path_delete);
    if(!dsc || !path)
      return;

    assert(std::holds_alternative<Point>(points[0]) && "First Segment can't be QuadPathSegment");

    const auto [startX, startY] = std::get<Point>(points[0]);
    const auto first_point = lv_fpoint_t { static_cast<float>(startX), static_cast<float>(startY) };
    lv_vector_path_move_to(path.get(), &first_point);

    for(size_t i = 1; i < points.size(); ++i)
    {
      if(std::holds_alternative<Point>(points[i]))
      {
        const auto [x, y] = std::get<Point>(points[i]);
        const auto fpoint = lv_fpoint_t { static_cast<float>(x), static_cast<float>(y) };
        lv_vector_path_line_to(path.get(), &fpoint);
      }
      else if(std::holds_alternative<QuadPathSegment>(points[i]))
      {
        const auto [a, b] = std::get<QuadPathSegment>(points[i]);
        lv_fpoint_t start = { static_cast<float>(a.x), static_cast<float>(a.y) };
        lv_fpoint_t end = { static_cast<float>(b.x), static_cast<float>(b.y) };
        lv_vector_path_quad_to(path.get(), &start, &end);
      }
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
      fillPolygon(stroke, fill, toPathSegments(points));
      return;
    }

    points = translate(std::move(points), m_currentOffset);

    if(auto bounds = getBoundingArea(points, stroke.width / 2 + rc.radius))
    {
      if(!clipAreaToPane(*bounds, m_layer._clip_area))
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
    const auto translatedArcOptions = translate(arcOptions, m_currentOffset);
    const auto halfStroke = static_cast<int>(std::ceil(static_cast<float>(translatedArcOptions.strokeWidth) / 2.0f));
    const auto radius = static_cast<int>(std::ceil(translatedArcOptions.radius)) + halfStroke;
    lv_area_t bounds { .x1 = translatedArcOptions.position.x - radius,
                       .y1 = translatedArcOptions.position.y - radius,
                       .x2 = translatedArcOptions.position.x + radius,
                       .y2 = translatedArcOptions.position.y + radius };

    if(!clipAreaToPane(bounds, m_layer._clip_area))
      return;

    const auto dsc = tVectorDscPtr(lv_vector_dsc_create(&m_layer), &lv_vector_dsc_delete);
    if(!dsc)
      return;

    const auto path = tVectorPathPtr(lv_vector_path_create(LV_VECTOR_PATH_QUALITY_HIGH), &lv_vector_path_delete);
    if(!path)
      return;

    const lv_fpoint_t center = { static_cast<float>(translatedArcOptions.position.x), static_cast<float>(translatedArcOptions.position.y) };

    lv_vector_path_append_arc(path.get(), &center, translatedArcOptions.radius, translatedArcOptions.startAngle, translatedArcOptions.sweepAngle, false);

    lv_vector_dsc_set_fill_opa(dsc.get(), LV_OPA_0);

    const auto &color = translatedArcOptions.color;

    lv_vector_dsc_set_stroke_color(dsc.get(), lv_color_make(color.r, color.g, color.b));
    lv_vector_dsc_set_stroke_opa(dsc.get(), static_cast<lv_opa_t>(color.a * static_cast<float>(LV_OPA_COVER)));
    lv_vector_dsc_set_stroke_width(dsc.get(), static_cast<float>(translatedArcOptions.strokeWidth));
    lv_vector_dsc_set_stroke_join(dsc.get(), LV_VECTOR_STROKE_JOIN_ROUND);
    lv_vector_dsc_set_stroke_cap(dsc.get(), LV_VECTOR_STROKE_CAP_ROUND);

    if(translatedArcOptions.dashes.has_value())
    {
      auto dashes = translatedArcOptions.dashes.value();
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

        const ArcDrawOptions lineArc
            = { .position = props.center, .color = props.lineColor, .radius = props.radius, .strokeWidth = props.strokeWidth, .startAngle = lineAnglePos, .sweepAngle = lineAngle };

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

    r = translate(r, m_currentOffset);
    auto clipArea = m_layer._clip_area;

    if(r.size.w > 0)
    {
      clipArea.x1 = std::max(clipArea.x1, r.pos.x);
      clipArea.x2 = std::min(clipArea.x2, r.pos.x + r.size.w - 1);
    }

    if(r.size.h > 0)
    {
      clipArea.y1 = std::max(clipArea.y1, r.pos.y);
      clipArea.y2 = std::min(clipArea.y2, r.pos.y + r.size.h - 1);
    }

    if(clipArea.x1 > clipArea.x2 || clipArea.y1 > clipArea.y2)
      return;

    const auto *canvas = reinterpret_cast<lv_canvas_t *>(&m_canvas);
    const auto *draw_buf = canvas->draw_buf;

    if(!draw_buf)
      return;

    flushLayer();

    const auto x = r.pos.x + startX;
    const auto y = r.pos.y + startY;

    font.draw(t.text, x, y, [&](int px, int py, unsigned char coverage) {
      if(px >= clipArea.x1 && px <= clipArea.x2 && py >= clipArea.y1 && py <= clipArea.y2)
        drawFontPixel(*draw_buf, c, px, py, coverage);
    });
  }

  void LVGLDrawContext::putBitmap(const Bitmap &image, Point p, std::optional<Color> colorOverride)
  {
    p = translate(p, m_currentOffset);

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

    const auto start_x = std::max(p.x, 0);
    const auto start_y = std::max(p.y, 0);
    const auto src_start_x = std::max(-p.x, 0);
    const auto src_start_y = std::max(-p.y, 0);

    if(start_x >= canvas_width || start_y >= canvas_height)
      return;

    const auto end_x = std::min<int>(p.x + src_width, static_cast<int>(canvas_width));
    const auto end_y = std::min<int>(p.y + src_height, static_cast<int>(canvas_height));
    const auto copy_width = end_x - start_x;
    const auto copy_height = end_y - start_y;

    if(copy_width <= 0 || copy_height <= 0)
      return;

    const auto canvas_data = static_cast<uint8_t *>(draw_buf->data);
    const uint8_t *src_data = image.start;

    for(int y = 0; y < copy_height; ++y)
    {
      const int canvas_y = start_y + y;
      const int src_y = src_start_y + y;

      uint8_t *canvas_row = canvas_data + canvas_y * canvas_stride + start_x * 4;
      const uint8_t *src_row = src_data + src_y * src_stride + src_start_x * 4;

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

  void LVGLDrawContext::drawSVG(const SVGPath &path, Rect dest, std::optional<Color> colorOverride)
  {
    if(const auto *doc = SVGDocumentCache::documentFor(path))
      renderDocumentToContext(*this, *doc, dest.pos, dest.size.w, dest.size.h, colorOverride);
  }

  void LVGLDrawContext::drawSVG(const SVGFileContent &content, Rect dest, std::optional<Color> colorOverride)
  {
    if(const auto *doc = SVGDocumentCache::documentFor(content))
      renderDocumentToContext(*this, *doc, dest.pos, dest.size.w, dest.size.h, colorOverride);
  }

  void LVGLDrawContext::drawText(const Glib::ustring &text, int x, int y, const FreeTypeFont &font, Color c)
  {
    if(text.empty())
      return;

    x += m_currentOffset.x;
    y += m_currentOffset.y;

    const auto *canvas = reinterpret_cast<lv_canvas_t *>(&m_canvas);
    const auto *draw_buf = canvas->draw_buf;
    if(!draw_buf)
      return;

    flushLayer();
    font.draw(text, x, y, [&](int px, int py, unsigned char coverage) { drawFontPixel(*draw_buf, c, px, py, coverage); });
  }

  void LVGLDrawContext::drawFontPixel(const lv_draw_buf_t &draw_buf, const Color &baseColor, int px, int py, unsigned char coverage)
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
}
