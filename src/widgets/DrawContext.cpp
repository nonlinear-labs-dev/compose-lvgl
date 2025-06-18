// #include <compose/widgets/DrawContext.h>
// #include <utility>
// #include <cairomm/context.h>
// #include <gtkmm/drawingarea.h>
//
// Compose::CairoDrawContext::CairoDrawContext(tCtx ctx)
//     : m_context(std::move(ctx))
// {
// }
//
// void Compose::CairoDrawContext::drawLine(const StrokeStyle style, const Point p1, const Point p2)
// {
//   m_context->begin_new_path();
//
//   auto [r, g, b, a] = style.color.normalized();
//
//   m_context->set_line_width(style.width);
//   m_context->set_source_rgba(r, g, b, a);
//
//   m_context->move_to(p1.x, p1.y);
//   m_context->line_to(p2.x, p2.y);
//
//   m_context->stroke();
// }
//
// void Compose::CairoDrawContext::strokeRect(const StrokeStyle style, const Rect rect)
// {
//   m_context->begin_new_path();
//   auto [r, g, b, a] = style.color.normalized();
//   m_context->set_line_width(style.width);
//
//   m_context->set_source_rgba(r, g, b, a);
//   m_context->rectangle(rect.pos.x, rect.pos.y, rect.size.w, rect.size.h);
//   m_context->stroke();
// }
//
// void Compose::CairoDrawContext::fillRect(const Color color, const Rect rect)
// {
//   m_context->begin_new_path();
//   auto [r, g, b, a] = color.normalized();
//   m_context->set_source_rgba(r, g, b, a);
//   m_context->rectangle(rect.pos.x, rect.pos.y, rect.size.w, rect.size.h);
//   m_context->fill();
// }
//
// void Compose::CairoDrawContext::fillRoundedRect(const Color color, const Rect rect, const RoundedCorner rc)
// {
//   const auto x = rect.pos.x;
//   const auto y = rect.pos.y;
//   const auto w = rect.size.w;
//   const auto h = rect.size.h;
//
//   const auto r1 = rc.topLeft;
//   const auto r2 = rc.topRight;
//   const auto r3 = rc.bottomRight;
//   const auto r4 = rc.bottomLeft;
//
//   m_context->begin_new_path();
//   auto [r, g, b, a] = color.normalized();
//   m_context->set_source_rgba(r, g, b, a);
//   m_context->move_to(x, y + r1);
//   // curve top-left (r1)
//   m_context->curve_to(x, y, x + r1, y, x + r1, y);
//   // top
//   m_context->line_to(x + w - r2, y);
//   // curve top-right (r2)
//   m_context->curve_to(x + w - r2, y, x + w , y, x + w, y + r2);
//   // right
//   m_context->line_to(x + w, y + h - r3);
//   // curve bottom-right (r3)
//   m_context->curve_to(x + w , y + h, x + w - r3, y + h, x + w - r3, y + h);
//   // bottom
//   m_context->line_to(x + r4, y + h);
//   // curve bottom-left (r4)
//   m_context->curve_to(x, y + h, x, y + h - r4, x , y + h - r4);
//   // left
//   m_context->line_to(x, y + r1);
//   m_context->close_path();
//   m_context->fill();
// }
//
// void Compose::CairoDrawContext::fillPolygon(StrokeStyle stroke, Color fill, std::vector<Point> points)
// {
//   if(points.size() < 3)
//   {
//     return;
//   }
//
//   m_context->begin_new_path();
//
//   auto [fr, fg, fb, fa] = fill.normalized();
//   m_context->set_source_rgba(fr, fg, fb, fa);
//
//   m_context->move_to(points[0].x, points[0].y);
//   for(size_t i = 1; i < points.size(); ++i)
//   {
//     m_context->line_to(points[i].x, points[i].y);
//   }
//   m_context->close_path();
//   m_context->fill_preserve();
//
//   auto [sr, sg, sb, sa] = stroke.color.normalized();
//   m_context->set_line_width(stroke.width);
//   m_context->set_source_rgba(sr, sg, sb, sa);
//   m_context->stroke();
// }
