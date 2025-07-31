#pragma once
#include "Widget.h"
#include "src/widgets/label/lv_label.h"

#include <functional>
#include <string>

namespace Compose
{
  class Label final : public Widget
  {
   public:
    using Widget::setModifier;

    using AutorunStringCB = std::function<std::string()>;

    template <typename... tArgs>
    explicit Label(BaseWidget &parent, tArgs... args)
        : Widget(lv_label_create(parent.getHandle()))
    {
      setModifier(Text { "" });
      applyDefaultStyle(BaseWidget::getHandle());
      lv_obj_set_size(BaseWidget::getHandle(), LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      setModifier(FlexGrow { 0 });
      setModifier(BackgroundColor { Color::TRANSPARENT() });
      (setModifier(args), ...);
    }

    explicit Label(WidgetType *handle);
    void operator<<(AutorunStringCB &&cb) const;

    void setModifier(TextAlign a) const
    {
      lv_obj_set_style_text_align(getHandle(), a.it, LV_PART_MAIN);
    }

    void setModifier(Text s) const;
    void setModifier(PrimaryColor s) const override;
    void setModifier(Font s) const;
  };

  struct FontStorage
  {
    using tPathBuilder = std::function<std::string(Font)>;

    explicit FontStorage(tPathBuilder builder)
        : buildPath(std::move(builder))
        , fonts()
    {
      nltools::Log::error("build font storage");
    }

    FontStorage(const FontStorage &) = delete;             // no copy
    FontStorage &operator=(const FontStorage &) = delete;  // no assignment

    struct FontWrapper
    {
      explicit FontWrapper(const std::string &path)
          : m_font { lv_binfont_create(std::format("S:{}", path).c_str()) }
      {
      }

      ~FontWrapper()
      {
        lv_binfont_destroy(m_font);
      }

      lv_font_t *m_font;
    };

    FontWrapper &getFont(const Font &font)
    {
      auto it = fonts.find(font);
      if(it == fonts.end())
      {
        it = fonts.emplace(font, std::make_unique<FontWrapper>(buildPath(font))).first;
      }
      return *it->second;
    }

    tPathBuilder buildPath;
    std::unordered_map<Font, std::unique_ptr<FontWrapper>> fonts;
  };

  extern std::unique_ptr<FontStorage> s_fontStorage;
}

#define LABEL(...) it.add(Compose::Label(it __VA_OPT__(, __VA_ARGS__))) << [=](Compose::Label &&it)
#define USE_FONT_STORAGE(...) Compose::s_fontStorage = std::make_unique<Compose::FontStorage>(__VA_ARGS__);