#include <format>
#include <filesystem>
#include <stdexcept>

#define SDL_MAIN_HANDLED

#include "VirtualListDemoApp.h"
#include "compose/widgets/Application.h"
#include "compose/widgets/Button.h"
#include "compose/widgets/Label.h"
#include "compose/widgets/container/HorizontalList.h"
#include "compose/widgets/container/VerticalList.h"

using namespace Compose;

static std::string findDemoFontPath(const Font &font)
{
  const auto fileName = std::format("{}-{}.ttf", font.baseName, Font::getWeightString(font.weight));
  const auto candidate = std::filesystem::path(__FILE__).parent_path() / fileName;

  if(std::filesystem::exists(candidate))
  {
    return candidate.string();
  }

  throw std::invalid_argument(std::string("Could not find font: ") + candidate.string());
}

int runVirtualListDemo()
{
  constexpr int c_itemCount = 10000;
  USE_FONT_STORAGE(findDemoFontPath);

  auto selectedItem = std::make_shared<Reactive::Var<int>>(0);

  APPLICATION({ 32, 32, 640, 500 })
  {
    COLUMN(SizePercentage::FULL(), BackgroundColor { Color::BLACK() })
    {
      LABEL(FixedSize { 200, 32 }, PrimaryColor { Color::WHITE() }, TextAlign::LEFT(), VerticalAlign::CENTER(),
            Padding { 10, 0, 0, 0 })
      {
        TEXT("Virtual list demo with 10000 items");
      };

      constexpr auto c_itemHeight = 28;

      VERTICAL_LIST(ItemCount { c_itemCount }, Height::PX(220), Width::FULL(), BackgroundColor { Color::BLACK() })
      {
        ITEM_HEIGHT(c_itemHeight);
        SCROLL_TO_ITEM(selectedItem->get());
        LIST_ITEM(int idx)
        {
          HEIGHT(c_itemHeight);
          FLEX_GROW(0);
          PADDING(10, 0, 10, 0);
          BACKGROUND_COLOR(idx % 2 == 0 ? Color::RGB(16, 16, 16) : Color::RGB(24, 24, 24));

          LABEL(SizePercentage::FULL(), TextAlign::LEFT(), VerticalAlign::CENTER(),
                PrimaryColor { idx % 10 == 0 ? Color::GREEN() : Color::WHITE() })
          {
            TEXT(std::format("Item {}", idx));
          };
        };
      };

      HORIZONTAL_LIST(ItemCount { c_itemCount }, Height::PX(80), Width::FULL(), BackgroundColor { Color::BLACK() })
      {
        ITEM_WIDTH(120);
        SCROLL_TO_ITEM(selectedItem->get());
        LIST_ITEM(int idx)
        {
          WIDTH(120);
          FLEX_GROW(0);
          PADDING(10, 0, 10, 0);
          BACKGROUND_COLOR(idx % 2 == 0 ? Color::RGB(36, 36, 36) : Color::RGB(52, 52, 52));

          LABEL(SizePercentage::FULL(), TextAlign::CENTER(), VerticalAlign::CENTER(),
                PrimaryColor { idx % 10 == 0 ? Color::GREEN() : Color::WHITE() })
          {
            TEXT(std::format("Item {}", idx));
          };
        };
      };

      ROW(Height { 50 }) { };

      ROW()
      {
        BUTTON(FixedSize { 100, 100 })
        {
          TEXT("Previous");

          PRESSED(Position)
          {
            *selectedItem = std::max(0, *selectedItem.get() - 1);
            return true;
          };
        };

        LABEL(FixedSize { 100, 100 })
        {
          TEXT("Item:" + std::to_string(selectedItem->get()));
        };

        BUTTON(FixedSize { 100, 100 })
        {
          TEXT("Next");
          PRESSED(Position)
          {
            *selectedItem = *selectedItem.get() + 1;
            return true;
          };
        };
      };

      ROW(Height { 50 })
      {
        LABEL(Height { 50 }, VerticalAlign::CENTER())
        {
          TEXT("Drag me!");
        };
      };
    };
  };

  return 0;
}
