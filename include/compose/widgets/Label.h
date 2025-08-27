#pragma once
#include "CustomDrawingElement.h"
#include "compose/FreeTypeFont.h"

#include <functional>
#include <string>
#include <memory>

namespace Compose
{
  class Label : public Widget
  {
   public:
    using Widget::setModifier;

    using AutorunStringCB = std::function<std::string()>;

    void setRenderCallback() const;
    template <typename... tArgs>
    explicit Label(BaseWidget &parent, tArgs... args)
        : Widget(lv_canvas_create(parent.getHandle()))
    {
      setModifier(Text { "" });
      applyDefaultStyle(BaseWidget::getHandle());
      lv_obj_set_size(BaseWidget::getHandle(), LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      setModifier(FlexGrow { 0 });
      Label::setModifier(BackgroundColor { Color::TRANSPARENT() });
      Label::setModifier(PrimaryColor { Color::WHITE() });
      (setModifier(args), ...);
      setRenderCallback();
    }

    explicit Label(WidgetType *handle);
    void operator<<(AutorunStringCB &&cb) const;

    void setModifier(TextAlign a) const
    {
      lv_obj_set_style_text_align(getHandle(), a.it, LV_PART_MAIN);
    }

    template <typename T> T &getModifier() const
    {
      return ensureDataForKeyExistsOwning<T>(typeid(T).name());
    }

    void setModifier(const Text &s) const;
    void setModifier(PrimaryColor s) const override;
    void setModifier(BackgroundColor c) const override;
    virtual void setModifier(Font s) const;

    void cleanup() const;

    void clear() override
    {
      cleanup();
      Widget::clear();
    }

   private:
    void setDrawCall(CustomDrawingElement::tDrawCB &&draw) const;
    struct
    {
      void operator<<(CustomDrawingElement::tDrawCB &&cb) const
      {
        m_parent->setDrawCall(std::move(cb));
      }

      Label *m_parent;
    } render { this };
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

    FontStorage(const FontStorage &) = delete;
    FontStorage &operator=(const FontStorage &) = delete;

    FreeTypeFont &getFont(const Font &font)
    {
      auto it = fonts.find(font);
      if(it == fonts.end())
      {
        it = fonts.emplace(font, std::make_unique<FreeTypeFont>(buildPath(font), font.size)).first;
      }
      return *it->second;
    }

    tPathBuilder buildPath;
    std::unordered_map<Font, std::unique_ptr<FreeTypeFont>> fonts;
  };

  extern std::unique_ptr<FontStorage> s_fontStorage;
}

#define LABEL(...) it.add(Compose::Label(it __VA_OPT__(, __VA_ARGS__))) << [=](Compose::Label &&it)
#define USE_FONT_STORAGE(...) Compose::s_fontStorage = std::make_unique<Compose::FontStorage>(__VA_ARGS__);