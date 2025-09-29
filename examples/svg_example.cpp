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
    Application app;
    
    Window window(app, Size{800, 600}, BackgroundColor{Color{0.1f, 0.1f, 0.1f, 1.0f}});
    
    // Create an SVG renderer
    SVG_RENDERER(Position{100, 100}, Size{200, 200}) << [=](SVGRenderer&& svg)
    {
        // Example SVG content - a simple red circle
        std::string svgContent = R"(
            <svg width="200" height="200" xmlns="http://www.w3.org/2000/svg">
                <circle cx="100" cy="100" r="80" fill="red" stroke="black" stroke-width="2"/>
                <text x="100" y="110" text-anchor="middle" fill="white" font-size="16">Hello SVG!</text>
            </svg>
        )";
        
        svg.setSVG(svgContent);
    };
    
    // Another SVG renderer with a different shape
    SVG_RENDERER(Position{350, 100}, Size{200, 200}) << [=](SVGRenderer&& svg)
    {
        // Example SVG content - a blue rectangle with rounded corners
        std::string svgContent = R"(
            <svg width="200" height="200" xmlns="http://www.w3.org/2000/svg">
                <rect x="20" y="20" width="160" height="160" rx="20" ry="20" fill="blue" stroke="white" stroke-width="3"/>
                <polygon points="100,50 150,100 100,150 50,100" fill="yellow" stroke="black" stroke-width="2"/>
            </svg>
        )";
        
        svg.setSVG(svgContent);
    };
    
    // You can also load SVG from a file
    // SVG_RENDERER(Position{100, 350}, Size{200, 200}) << [=](SVGRenderer&& svg)
    // {
    //     svg.setSVGFile("/path/to/your/svg/file.svg");
    // };
    
    return app.run();
}
