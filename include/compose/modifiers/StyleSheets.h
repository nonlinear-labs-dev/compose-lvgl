#pragma once

#include "BackgroundColor.h"
#include "Border.h"
#include "FlexFlow.h"
#include "Font.h"
#include "Margin.h"
#include "Modifiers.h"
#include "Padding.h"
#include "PrimaryColor.h"
#include "RoundedCorner.h"
#include "Scrollable.h"
#include "tools/json.h"

#include <optional>
#include <string_view>
#include <string>
#include <tuple>
#include <type_traits>
#include <functional>
#include <stdexcept>

#include "StyleSheetJson.h"

namespace Compose
{
  struct StyleSheet;

  struct StyleClass
  {
    std::string name;
  };

  template <typename... T> struct MakeOptional;
  template <typename... T> struct MakeOptional<std::tuple<T...>>
  {
    using type = std::tuple<std::optional<T>...>;
  };

  struct Style
  {
    using Properties = std::tuple<BackgroundColor, PrimaryColor, Font, TextAlign, VerticalAlign, FlexAlign, FlexFlow, Expand, Width, Height, Margin, MarginLeft, MarginRight, MarginTop, MarginBottom,
                                  Padding, Border, BorderWidth, BorderColor, BorderSides, RoundedCorner, Scrollable>;

    MakeOptional<Properties>::type properties;
    std::vector<const StyleSheet*> sheets;
    bool debug = false;

    template <typename P> void set(const P& p)
    {
      if(debug)
      {
        nlohmann::json j = p;
        printf("Style::set: setting property %s = %s\n", typeid(P).name(), j.dump().c_str());
      }
      std::get<std::optional<P>>(properties) = p;
    }

    template <typename... P> void copyFrom(const Style& other, std::tuple<P...>)
    {
      ((std::get<std::optional<P>>(properties) = std::get<std::optional<P>>(other.properties)), ...);
    }

    template <typename T> T get(const T& def) const
    {
      return std::get<std::optional<T>>(properties).value_or(def);
    }

    Style& add(auto... names);
    Style add(auto... names) const;
    Style inherit(auto... names) const;
    Style inheritLowPrio(auto... names) const;
    Style inherit() const;
    [[nodiscard]] nlohmann::json var(const std::string& name) const;

    template <typename T> [[nodiscard]] T var(const std::string& name) const
    {
      return var(name).template get<T>();
    }

    [[nodiscard]] std::string dump(bool onlyNonNullOptProperties = true) const;

   private:
    [[nodiscard]] std::vector<const StyleSheet*> processClasses(auto... names);
    void prependSheets(std::vector<const StyleSheet*>&&);
    void appendSheets(std::vector<const StyleSheet*>&&);
  };

  struct StyleSheet
  {
    static std::size_t calcNameHash(std::string_view value)
    {
      return std::hash<std::string_view> { }(value);
    }

    template <typename... Args>
    StyleSheet(const std::string& name, Args&&... args)
        : name(name)
        , nameHash(calcNameHash(this->name))
    {
      (apply(std::move(args)), ...);
    }

    template <typename Arg> void apply(Arg&& a)
    {
      std::get<std::optional<std::decay_t<Arg>>>(styles) = a;
    }

    void apply(std::unique_ptr<StyleSheet>&& a)
    {
      children.emplace_back(std::move(a));
      children.back()->parent = this;
    }

    std::string name;
    std::size_t nameHash;
    MakeOptional<Style::Properties>::type styles;
    std::unordered_map<std::string, nlohmann::json> vars;
    std::vector<std::unique_ptr<StyleSheet>> children;
    StyleSheet* parent = nullptr;
  };

  StyleSheet styleSheet(const std::string& name, auto... p)
  {
    return StyleSheet { name, p... };
  }

  std::vector<const StyleSheet*> Style::processClasses(auto... classes)
  {
    std::vector<const StyleSheet*> nestedSheets;

    if constexpr(sizeof...(classes) > 0)
    {
      auto doNothing = [] { };
      for(const auto& name : { std::string_view { classes }... })
      {
        const std::string_view requestedName = name;
        const auto requestedHash = StyleSheet::calcNameHash(requestedName);

        for(auto* sheet : sheets)
        {
          for(const auto& nestedSheet : sheet->children)
          {
            if(nestedSheet->nameHash == requestedHash && nestedSheet->name == requestedName)
            {
              if(debug)
              {
                printf("Style::processClasses: found matching style sheet %s\n", nestedSheet->name.c_str());
                printf("parents:\n");
                auto parent = nestedSheet->parent;
                while(parent)
                {
                  printf("\tStyle::processClasses: parent: %s\n", parent->name.c_str());
                  parent = parent->parent;
                }
                printf("\n\n\n");
              }

              if(std::count(sheets.begin(), sheets.end(), nestedSheet.get()) == 0)
              {
                nestedSheets.push_back(nestedSheet.get());

                if(debug)
                {
                  printf("Style::processClasses: adding nested style sheet to lookup scope %s\n", nestedSheet->name.c_str());
                }
              }

              std::apply([&](const auto&... a) { ((a.has_value() ? set(a.value()) : doNothing()), ...); }, nestedSheet->styles);
            }
          }
        }
      }
    }

    return nestedSheets;
  }

  inline void Style::prependSheets(std::vector<const StyleSheet*>&& nested)
  {
    for(auto* sheet : nested)
      if(std::count(sheets.begin(), sheets.end(), sheet) == 0)
        sheets.insert(sheets.begin(), sheet);
  }

  inline void Style::appendSheets(std::vector<const StyleSheet*>&& nested)
  {
    for(auto* sheet : nested)
      if(std::count(sheets.begin(), sheets.end(), sheet) == 0)
        sheets.push_back(sheet);
  }

  Style& Style::add(auto... classes)
  {
    appendSheets(processClasses(classes...));
    return *this;
  }

  Style Style::add(auto... classes) const
  {
    Style ret = *this;
    ret.appendSheets(ret.processClasses(classes...));
    return ret;
  }

  Style Style::inherit(auto... classes) const
  {
    Style ret { .sheets = sheets, .debug = debug };
    ret.appendSheets(ret.processClasses(classes...));
    return ret;
  }

  Style Style::inheritLowPrio(auto... classes) const
  {
    Style ret { .sheets = sheets };
    ret.prependSheets(ret.processClasses(classes...));
    return ret;
  }

  inline Style Style::inherit() const
  {
    return { .sheets = sheets };
  }

  inline nlohmann::json Style::var(const std::string& name) const
  {
    const nlohmann::json* result = nullptr;

    for(const auto* sheet : sheets)
    {
      if(const auto it = sheet->vars.find(name); it != sheet->vars.end())
        result = &it->second;
    }

    if(result)
      return *result;

    throw std::invalid_argument("style variable not found: " + name);
  }
}

#define STYLECLASS(...) it.doAutorun([=] { it.setModifier(StyleClass { __VA_ARGS__ }); });
