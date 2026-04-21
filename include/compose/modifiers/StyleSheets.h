#pragma once

#include "BackgroundColor.h"
#include "Border.h"
#include "Font.h"
#include "Margin.h"
#include "Modifiers.h"
#include "Padding.h"
#include "PrimaryColor.h"
#include "RoundedCorner.h"
#include "Scrollable.h"

#include <optional>
#include <string_view>
#include <string>
#include <tuple>
#include <type_traits>
#include <functional>

namespace Compose
{
  struct StyleSheet;

  template <typename... T> struct MakeOptional;
  template <typename... T> struct MakeOptional<std::tuple<T...>>
  {
    using type = std::tuple<std::optional<T>...>;
  };

  struct Style
  {
    using Properties = std::tuple<BackgroundColor, PrimaryColor, Font, TextAlign, VerticalAlign, FlexAlign, Width, Height, Margin, MarginLeft, MarginRight, MarginTop, MarginBottom,
                                  Padding, Border, BorderWidth, BorderColor, BorderSides, RoundedCorner, Scrollable>;

    MakeOptional<Properties>::type properties;
    std::vector<const StyleSheet*> sheets;

    template <typename P> void set(const P& p)
    {
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
    Style inherit() const;
    [[nodiscard]] std::string dump(bool onlyNonNullOptProperties = true) const;

   private:
    void processClasses(auto... names);
  };

  struct StyleSheet
  {
    static std::size_t calcNameHash(std::string_view value)
    {
      return std::hash<std::string_view> {}(value);
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

    void apply(StyleSheet&& a)
    {
      children.emplace_back(std::forward<StyleSheet>(a));
    }

    std::string name;
    std::size_t nameHash;
    MakeOptional<Style::Properties>::type styles;
    std::vector<StyleSheet> children;
  };

  StyleSheet styleSheet(const std::string& name, auto... p)
  {
    return StyleSheet { name, p... };
  }

  void Style::processClasses(auto... classes)
  {
    std::vector<const StyleSheet*> nestedSheets;

    auto doNothing = [] {};
    for(const auto& name : { std::string_view { classes }... })
    {
      const std::string_view requestedName = name;
      const auto requestedHash = StyleSheet::calcNameHash(requestedName);

      for(auto* sheet : sheets)
      {
        for(const auto& nestedSheet : sheet->children)
        {
          if(nestedSheet.nameHash == requestedHash && nestedSheet.name == requestedName)
          {
            if(std::count(sheets.begin(), sheets.end(), &nestedSheet) == 0)
              nestedSheets.push_back(&nestedSheet);

            std::apply([&](const auto&... a) { ((a.has_value() ? set(a.value()) : doNothing()), ...); }, nestedSheet.styles);
          }
        }
      }
    }

    for(auto* sheet : nestedSheets)
      if(std::count(sheets.begin(), sheets.end(), sheet) == 0)
        sheets.push_back(sheet);
  }

  Style& Style::add(auto... classes)
  {
    processClasses(classes...);
    return *this;
  }
  Style Style::add(auto... classes) const
  {
    Style ret = *this;
    ret.processClasses(classes...);
    return ret;
  }

  Style Style::inherit(auto... classes) const
  {
    Style ret { .sheets = sheets };
    ret.processClasses(classes...);
    return ret;
  }

  inline Style Style::inherit() const
  {
    return { .sheets = sheets };
  }
}
