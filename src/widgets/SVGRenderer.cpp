#include "compose/widgets/SVGRenderer.h"
#include "compose/widgets/DrawContext.h"
#include <fstream>
#include <sstream>

namespace Compose
{
  void SVGRenderer::setModifier(SVGFileContent svgContent) const
  {
    auto document = lunasvg::Document::loadFromData(svgContent.content);

    if(document)
    {
      const Widget widget(getHandle());

      auto& svgData = widget.ensureDataForKeyExistsOwning<SVGData>(
          c_svgData, [handle = getHandle(), cb = [](DrawContext&, int, int) {}] { return new SVGData(handle, cb); });

      //svgData.document.modify([&document](auto& doc) { doc = std::move(document); });

      setDrawCall(
          [handle = getHandle()](DrawContext& ctx, int width, int height)
          {
            Widget widget(handle);
            auto& svgData = widget.ensureDataForKeyExistsOwning<SVGData>(
                c_svgData, [handle, cb = [](DrawContext&, int, int) {}] { return new SVGData(handle, cb); });
            svgData.renderToDrawContext(ctx, width, height);
          });
    }
  }

  void SVGRenderer::setModifier(SVGPath path) const
  {
    std::ifstream file(path.it);
    if(!file.is_open())
      return;

    std::stringstream buffer;
    buffer << file.rdbuf();
    setModifier(SVGFileContent { buffer.str() });
  }

  void SVGRenderer::setModifier(PrimaryColor col) const
  {
    auto& svgData = ensureDataForKeyExistsOwning<SVGData>(
        c_svgData, [handle = getHandle(), cb = [](DrawContext&, int, int) {}] { return new SVGData(handle, cb); });
    svgData.color = col;
  }
}
