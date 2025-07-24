#include <compose/widgets/DrawContext.h>
#include "src/draw/lv_draw.h"
#include "src/misc/lv_color.h"
#include "src/misc/lv_area.h"
#include <src/widgets/canvas/lv_canvas.h>

namespace Compose
{
  LVGLDrawContext::LVGLDrawContext(tCanvas ctx)
      : m_layer {}
      , m_canvas(ctx)
  {
    lv_canvas_init_layer(m_canvas, &m_layer);
  }

  LVGLDrawContext::~LVGLDrawContext()
  {
    lv_canvas_finish_layer(m_canvas, &m_layer);
  }

  void LVGLDrawContext::drawLine(const StrokeStyle style, const Point p1, const Point p2)
  {
    if(!m_canvas)
      return;

    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    line_dsc.p1.x = p1.x;
    line_dsc.p1.y = p1.y;
    line_dsc.p2.x = p2.x;
    line_dsc.p2.y = p2.y;
    line_dsc.color = lv_color_make(style.color.r, style.color.g, style.color.b);
    line_dsc.width = style.width;
    line_dsc.opa = static_cast<lv_opa_t>(style.color.a * 255.0);

    lv_draw_line(&m_layer, &line_dsc);
  }

  void LVGLDrawContext::strokeRect(const StrokeStyle style, const Rect rect)
  {
    if(!m_canvas)
      return;

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

  void LVGLDrawContext::fillRect(const Color color, const Rect rect)
  {
    if(!m_canvas)
      return;

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
#warning "this does not do what it says!"
    if(!m_canvas)
      return;

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

  void LVGLDrawContext::fillPolygon(StrokeStyle stroke, Color fill, std::vector<Point> points)
  {
    if(points.size() < 3 || !m_canvas)
      return;

    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_color = lv_color_make(fill.r, fill.g, fill.b);
    rect_dsc.bg_opa = static_cast<lv_opa_t>(fill.a * 255.0);
    rect_dsc.border_color = lv_color_make(stroke.color.r, stroke.color.g, stroke.color.b);
    rect_dsc.border_width = stroke.width;
    rect_dsc.border_opa = static_cast<lv_opa_t>(stroke.color.a * 255.0);

    lv_area_t area;
    area.x1 = points[0].x;
    area.y1 = points[0].y;
    area.x2 = points[0].x;
    area.y2 = points[0].y;

    for(const auto &point : points)
    {
      area.x1 = LV_MIN(area.x1, point.x);
      area.y1 = LV_MIN(area.y1, point.y);
      area.x2 = LV_MAX(area.x2, point.x);
      area.y2 = LV_MAX(area.y2, point.y);
    }

    lv_draw_rect(&m_layer, &rect_dsc, &area);
  }
}
