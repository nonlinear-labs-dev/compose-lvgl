#include "compose/state/CanvasData.h"

#include <compose/widgets/Label.h>
#include <compose/widgets/DrawContext.h>
#include <compose/widgets/LabelShared.h>
#include <compose/FreeTypeFont.h>
#include <utility>

namespace Compose
{
  void Label::setLabelRenderingFunction() const
  {
    setDrawCall(
        [handle = getHandle()](DrawContext &ctx, int w, int h)
        {
          const Label labelWidget(handle);
          const auto &cd = labelWidget.getDataForKey<LabelData>(c_labelData);

          if(cd.bgColor.get().a > 0)
            ctx.fillRect(cd.bgColor, { 0, 0, w, h });

          ctx.drawText(cd.text, cd.font, { 0, 0, w, h }, cd.primaryColor, cd.align, cd.verticalAlign);
        });
  }

  void Label::setModifier(Text t) const
  {
    LabelShared::setText(*this, t);
  }

  void Label::setModifier(PrimaryColor c) const
  {
    LabelShared::setPrimaryColor(*this, c);
  }

  void Label::setModifier(BackgroundColor c) const
  {
    LabelShared::setBackgroundColor(*this, c);
  }

  void Label::setModifier(Font s) const
  {
    LabelShared::setFont(*this, s);
  }

  void Label::setModifier(TextAlign a) const
  {
    LabelShared::setTextAlign(*this, a);
  }

  void Label::setModifier(VerticalAlign v) const
  {
    LabelShared::setVerticalAlign(*this, v);
  }

  void Label::setDrawCall(CustomDrawingElement::tDrawCB &&draw) const
  {
    LabelShared::setDrawCallCommon(*this, std::move(draw));
  }

  void Label::setModifier(Width w) const
  {
    LabelShared::setWidth(*this, w);
  }

  void Label::setModifier(Height h) const
  {
    LabelShared::setHeight(*this, h);
  }

  void Label::operator<<(AutorunStringCB &&cb) const
  {
    doAutorun([cb = std::move(cb), label = getHandle()] { Label(label).setModifier(Text { cb() }); });
  }

}