#include <compose/widgets/DrawContext.h>
#include "src/draw/lv_draw.h"
#include "src/misc/lv_color.h"
#include "src/misc/lv_area.h"
#include <src/widgets/canvas/lv_canvas.h>
#include "src/draw/lv_draw_vector.h"
#include <memory>
#include <algorithm>
#include "compose/widgets/Label.h"
#include "src/widgets/canvas/lv_canvas_private.h"

namespace Compose
{
  std::unique_ptr<FontStorage> s_fontStorage = nullptr;

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

  void LVGLDrawContext::drawText(Text t, Font f, Rect r, Color c, TextAlign ta)
  {
    if(!m_canvas || t.text.empty())
      return;

    if(!s_fontStorage)
      return;

    auto &font = s_fontStorage->getFont(f);
    const auto textWidth = static_cast<int32_t>(font.getStringWidth(t.text));

    const auto startX = [&]() -> int
    {
      switch(ta.it)
      {
        case LV_TEXT_ALIGN_LEFT:
          return r.pos.x;
        case LV_TEXT_ALIGN_RIGHT:
          return r.pos.x + r.size.w - textWidth;
        default:
        case LV_TEXT_ALIGN_CENTER:
        case LV_TEXT_ALIGN_AUTO:
          return r.pos.x + (r.size.w - textWidth) / 2;
      }
    }();

    font.draw(t.text, startX, r.pos.y,
              [&](auto x, auto y, auto value)
              {
                const auto factor = value / 255.0;
                auto pixelColor = c;
                pixelColor.a = factor * c.a;
                fillRect(pixelColor, { x, y, 1, 1 });
              });
  }

  void LVGLDrawContext::putBitmap(const Bitmap &image, Point p, std::optional<Color> colorOverride)
  {
    if(!m_canvas || !image.start || image.width <= 0 || image.height <= 0)
      return;

    auto *canvas = reinterpret_cast<lv_canvas_t *>(m_canvas);
    auto *draw_buf = canvas->draw_buf;

    if(!draw_buf || draw_buf->header.cf != LV_COLOR_FORMAT_ARGB8888)
      return;

    const int canvas_width = draw_buf->header.w;
    const int canvas_height = draw_buf->header.h;
    const int canvas_stride = draw_buf->header.stride;

    const int src_width = image.width;
    const int src_height = image.height;
    const int src_stride = image.stride;

    const int start_x = p.x;
    const int start_y = p.y;

    if(start_x >= canvas_width || start_y >= canvas_height)
      return;

    const int end_x = std::min(start_x + src_width, canvas_width);
    const int end_y = std::min(start_y + src_height, canvas_height);
    const int copy_width = end_x - start_x;
    const int copy_height = end_y - start_y;

    if(copy_width <= 0 || copy_height <= 0)
      return;

    uint8_t *canvas_data = (uint8_t *) draw_buf->data;
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
            final_a = static_cast<uint8_t>(override_color.a * src_a);
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

    lv_obj_invalidate(m_canvas);
  }
}
