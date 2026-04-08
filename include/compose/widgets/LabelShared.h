#pragma once
#include "DrawContext.h"
#include <compose/state/LabelData.h>
#include <compose/FreeTypeFont.h>

namespace Compose::LabelShared
{
  template <typename T> void setText(const T& self, Text t)
  {
    self.template getDataForKey<LabelData>(BaseWidget::c_labelData).text = t;
  }

  template <typename T> void setPrimaryColor(const T& self, PrimaryColor c)
  {
    self.template getDataForKey<LabelData>(BaseWidget::c_labelData).primaryColor = c;
  }

  template <typename T> void setBackgroundColor(const T& self, BackgroundColor c)
  {
    self.template getDataForKey<LabelData>(BaseWidget::c_labelData).bgColor = c;
  }

  template <typename T> void setFont(const T& self, Font f)
  {
    self.template getDataForKey<LabelData>(BaseWidget::c_labelData).font = f;
  }

  template <typename T> void setTextAlign(const T& self, TextAlign a)
  {
    self.template getDataForKey<LabelData>(BaseWidget::c_labelData).align = a;
  }

  template <typename T> void setVerticalAlign(const T& self, VerticalAlign v)
  {
    self.template getDataForKey<LabelData>(BaseWidget::c_labelData).verticalAlign = v;
  }

  template <typename T> void setWidth(const T& self, Width w)
  {
    self.template getDataForKey<LabelData>(BaseWidget::c_labelData).width = w;
    static_cast<const Widget&>(self).Widget::setModifier(w);
  }

  template <typename T> void setHeight(const T& self, Height h)
  {
    self.template getDataForKey<LabelData>(BaseWidget::c_labelData).height = h;
    static_cast<const Widget&>(self).Widget::setModifier(h);
  }

  template <typename T> void setDrawCallCommon(const T& self, CustomDrawingElement::tDrawCB&& draw)
  {
    nltools_detailedAssertAlways(!self.template doesDataForKeyExist<LabelData>(),
                                 "CanvasData should not exist, setting a new render callback is prohibited");
    Widget(self.getHandle())
        .doAutorun(
            [draw = std::move(draw), handle = self.getHandle()]
            {
              const Widget widget(handle);

              auto& canvasData = widget.ensureDataForKeyExistsOwning<LabelData>(
                  BaseWidget::c_labelData, [handle, d = draw] { return new LabelData(handle, d); });

              const auto w = lv_obj_get_width(handle);
              const auto h = lv_obj_get_height(handle);

              auto& bufferUser = canvasData.buffer.get();

              LVGLDrawContext drawContext(*handle);
              try
              {
                canvasData.drawCallback(drawContext, w, h);
              }
              catch(std::exception&)
              {
              }
            });
  }

  inline int computeStartX(int availableWidth, int textWidth, const TextAlign& a)
  {
    switch(a.it)
    {
      case LV_TEXT_ALIGN_LEFT:
        return 0;
      case LV_TEXT_ALIGN_RIGHT:
        return availableWidth - textWidth;
      default:
      case LV_TEXT_ALIGN_CENTER:
      case LV_TEXT_ALIGN_AUTO:
        return (availableWidth - textWidth) / 2;
    }
  }

  inline int computeStartYSingle(int availableHeight, const FreeTypeFont& font, const Glib::ustring& text,
                                 const VerticalAlign& a)
  {
    const auto bounds = font.getTextBounds(text);
    const auto textHeight = bounds.height();

    switch(a.it)
    {
      case VerticalAlign::Top:
        return -bounds.top;
      case VerticalAlign::Bottom:
        return availableHeight - bounds.bottom;
      default:
      case VerticalAlign::Center:
        return (availableHeight - textHeight) / 2 - bounds.top;
    }
  }

  inline int computeStartYBlock(int availableHeight, int totalTextHeight, int lineHeight, const FreeTypeFont& font,
                                const VerticalAlign& a)
  {
    switch(a.it)
    {
      case VerticalAlign::Top:
        return 0;
      case VerticalAlign::Bottom:
        return availableHeight - totalTextHeight;
      default:
      case VerticalAlign::Center:
        return (availableHeight - totalTextHeight) / 2 + font.getCapHeightPx() / 2 - lineHeight;
    }
  }
}
