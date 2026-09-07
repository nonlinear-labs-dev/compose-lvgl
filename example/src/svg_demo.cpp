#include "compose/widgets/Application.h"
#include "compose/widgets/Window.h"
#include "compose/widgets/SVGRenderer.h"
#include "compose/modifiers/Size.h"
#include "compose/modifiers/Position.h"
#include "compose/modifiers/BackgroundColor.h"
#include "compose/modifiers/PrimaryColor.h"

using namespace Compose;

int main()
{
  APPLICATION({0, 0, 800, 600})
  {
    COLUMN(SizePercentage::FULL(), LayoutType::none(), BackgroundColor{Color::WHITE().dimBrightness(0.1f)})
    {
      static constexpr std::string_view redCircleSvgContent = R"(
<svg width="200" height="200" xmlns="http://www.w3.org/2000/svg">
    <circle cx="100" cy="100" r="80" fill="red" stroke="black" stroke-width="2"/>
    <text x="100" y="110" text-anchor="middle" fill="white" font-size="16">Hello SVG!</text>
</svg>
)";
      SVG_RENDERER(Position{100, 100}, FixedSize{200, 200}, SVGFileContent{std::string(redCircleSvgContent)}){};

      static constexpr std::string_view bluRectSvgContent = R"(
<svg width="200" height="200" xmlns="http://www.w3.org/2000/svg">
  <rect x="20" y="20" width="160" height="160" rx="20" ry="20" fill="blue" stroke="white" stroke-width="3"/>
  <polygon points="100,50 150,100 100,150 50,100" fill="yellow" stroke="black" stroke-width="2"/>
</svg>
        )";
      SVG_RENDERER(Position{350, 100}, FixedSize{200, 200}, SVGFileContent{std::string(bluRectSvgContent)}){};

      // You can also load SVG from a file
      // SVG_RENDERER(Position{100, 350}, FixedSize{200, 200}, SVGPath{"/path/to/your/svg/file.svg"}) {};
    };
  };
}
