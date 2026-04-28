#include "compose/widgets/SVGRenderer.h"
#include "compose/widgets/DrawContext.h"
#include <fstream>
#include <sstream>

namespace Compose
{
  void SVGRenderer::setModifier(const SVGFileContent& svgContent) const
  {
    auto document = lunasvg::Document::loadFromData(svgContent.content);

    if(document)
    {
      const Widget widget(getHandle());

      auto& svgData = widget.ensureDataForKeyExistsOwning<SVGData>(c_svgData, [cb = [](DrawContext&, int, int) { }]
                                                                   { return new SVGData(cb); });

      svgData.document.modify([&document](auto& doc) { doc = std::move(document); });

      setDrawCall(
          [handle = getHandle()](DrawContext& ctx, int width, int height)
          {
            const Widget widget(handle);
            const auto& svgData = widget.ensureDataForKeyExistsOwning<SVGData>(
                c_svgData, [cb = [](DrawContext&, int, int) { }] { return new SVGData(cb); });
            svgData.renderToDrawContext(ctx, width, height);
          });
    }
  }

  void SVGRenderer::setModifier(const SVGPath& path) const
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
    auto& svgData = ensureDataForKeyExistsOwning<SVGData>(c_svgData, [cb = [](DrawContext&, int, int) { }]
                                                          { return new SVGData(cb); });
    svgData.color = col;
  }
}
