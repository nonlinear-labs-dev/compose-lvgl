#include "compose/widgets/SVGRenderer.h"
#include "compose/widgets/DrawContext.h"
#include <fstream>
#include <sstream>

namespace Compose
{
  void SVGRenderer::setSVG(const std::string& svgContent)
  {
    auto start = std::chrono::high_resolution_clock::now();
    auto document = lunasvg::Document::loadFromData(svgContent);

    if(document)
    {
      Widget widget(getHandle());

      auto& svgData = widget.ensureDataForKeyExistsOwning<SVGData>(
          c_svgData, [handle = getHandle(), cb = [](DrawContext&, int, int) {}] { return new SVGData(handle, cb); });

      svgData.document.modify([&document](auto& doc) { doc.reset(document.release()); });

      setDrawCall(
          [handle = getHandle()](DrawContext& ctx, int width, int height)
          {
            Widget widget(handle);

            auto& svgData = widget.ensureDataForKeyExistsOwning<SVGData>(
                c_svgData, [handle, cb = [](DrawContext&, int, int) {}] { return new SVGData(handle, cb); });

            svgData.renderToDrawContext(ctx, width, height);
          });
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    nltools::Log::error("parsing took", elapsed.count(), "ms");
  }

  void SVGRenderer::setSVGFile(const std::string& filePath)
  {
    std::ifstream file(filePath);
    if(!file.is_open())
      return;

    std::stringstream buffer;
    buffer << file.rdbuf();
    setSVG(buffer.str());
  }

  void SVGRenderer::setModifier(PrimaryColor col) const
  {
    auto& svgData = ensureDataForKeyExistsOwning<SVGData>(
        c_svgData, [handle = getHandle(), cb = [](DrawContext&, int, int) {}] { return new SVGData(handle, cb); });
    svgData.color = col;
  }
}
