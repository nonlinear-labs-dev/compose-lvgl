#include <compose/widgets/DrawContext.h>
#include "src/draw/lv_draw.h"
#include "src/misc/lv_color.h"
#include "src/misc/lv_area.h"
#include <src/widgets/canvas/lv_canvas.h>
#include "src/draw/lv_draw_vector.h"

namespace Compose
{
  LVGLDrawContext::LVGLDrawContext(tCanvas ctx)
      : m_layer {}
      , m_canvas(ctx)
  {
    lv_canvas_init_layer(m_canvas, &m_layer);
    lv_canvas_fill_bg(m_canvas, lv_color_black(), LV_OPA_0);
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

    lv_vector_dsc_t *dsc = lv_vector_dsc_create(&m_layer);
    if(!dsc)
      return;

    lv_vector_path_t *path = lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM);
    if(!path)
    {
      lv_vector_dsc_delete(dsc);
      return;
    }

    lv_fpoint_t first_point = { static_cast<float>(points[0].x), static_cast<float>(points[0].y) };
    lv_vector_path_move_to(path, &first_point);

    for(size_t i = 1; i < points.size(); ++i)
    {
      lv_fpoint_t point = { static_cast<float>(points[i].x), static_cast<float>(points[i].y) };
      lv_vector_path_line_to(path, &point);
    }

    lv_vector_path_close(path);

    lv_vector_dsc_set_fill_color(dsc, lv_color_make(fill.r, fill.g, fill.b));
    lv_vector_dsc_set_fill_opa(dsc, static_cast<lv_opa_t>(fill.a * 255.0));
    lv_vector_dsc_set_stroke_color(dsc, lv_color_make(stroke.color.r, stroke.color.g, stroke.color.b));
    lv_vector_dsc_set_stroke_opa(dsc, static_cast<lv_opa_t>(stroke.color.a * 255.0));
    lv_vector_dsc_set_stroke_width(dsc, static_cast<float>(stroke.width));
    lv_vector_dsc_add_path(dsc, path);

    lv_draw_vector(dsc);

    lv_vector_path_delete(path);
    lv_vector_dsc_delete(dsc);
  }
}
