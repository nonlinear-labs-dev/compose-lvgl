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

namespace Compose
{
  std::unique_ptr<FontStorage> s_fontStorage = nullptr;

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

  void LVGLDrawContext::drawLine(const StrokeStyle style, const Point p1, const Point p2)
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

    lv_draw_line(&m_layer, &line_dsc);
  }

  void LVGLDrawContext::drawQuadraticBezier(const StrokeStyle style, const Point start, const Point control,
                                            const Point end)
  {
    constexpr int segments = 20;
    for(int i = 0; i < segments; ++i)
    {
      const float t0 = static_cast<float>(i) / segments;
      const float t1 = static_cast<float>(i + 1) / segments;

      const float s0 = 1.0f - t0;
      const float s1 = 1.0f - t1;

      const int x0 = s0 * s0 * start.x + 2 * s0 * t0 * control.x + t0 * t0 * end.x;
      const int y0 = s0 * s0 * start.y + 2 * s0 * t0 * control.y + t0 * t0 * end.y;

      const int x1 = s1 * s1 * start.x + 2 * s1 * t1 * control.x + t1 * t1 * end.x;
      const int y1 = s1 * s1 * start.y + 2 * s1 * t1 * control.y + t1 * t1 * end.y;

      drawLine(style, { x0, y0 }, { x1, y1 });
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

  using tVectorDscPtr = std::unique_ptr<lv_vector_dsc_t, decltype(&lv_vector_dsc_delete)>;
  using tVectorPathPtr = std::unique_ptr<lv_vector_path_t, decltype(&lv_vector_path_delete)>;

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

  void LVGLDrawContext::drawSegmentedArc(DrawContext &ctx, SegmentedArcDrawOptions props)
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

        ctx.fillArc(lineArc);
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

        ctx.fillArc(spaceArc);
      }
    }
  }

  void LVGLDrawContext::drawText(Text t, Font f, Rect r, Color c, TextAlign ta)
  {
    if(t.text.empty())
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

    const auto *canvas = reinterpret_cast<lv_canvas_t *>(&m_canvas);
    const auto *draw_buf = canvas->draw_buf;
    if(!draw_buf)
      return;

    font.draw(t.text, startX, r.pos.y,
              [&](int px, int py, unsigned char coverage)
              {
                const auto mod_cov = static_cast<unsigned char>(static_cast<float>(coverage) * c.a);
                drawFontPixel(*draw_buf, c, px, py, mod_cov);
              });
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

    font.draw(text, x, y,
              [&](int px, int py, unsigned char coverage)
              {
                const auto mod_cov = static_cast<unsigned char>(static_cast<float>(coverage) * c.a);
                drawFontPixel(*draw_buf, c, px, py, mod_cov);
              });
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

    const auto final_a = static_cast<uint8_t>(coverage * baseColor.a);
    if(final_a == 0)
      return;

    uint8_t *canvas_pixel = canvas_data + py * canvas_stride + px * 4;
    canvas_pixel[0] = baseColor.b;
    canvas_pixel[1] = baseColor.g;
    canvas_pixel[2] = baseColor.r;
    canvas_pixel[3] = final_a;
  }
}
