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
#include <string>
#include <tuple>
#include <type_traits>

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
    using Properties = std::tuple<BackgroundColor, PrimaryColor, Font, TextAlign, VerticalAlign, FlexAlign, Width, Height, Margin, MarginLeft, MarginRight, MarginTop, MarginBottom, Padding,
                                  Border, BorderSides, RoundedCorner, Scrollable>;
    using InheritableProperties = std::tuple<PrimaryColor, Font, TextAlign>;

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

    Style inherit(auto... names) const;
    Style add(auto... names) const;
    [[nodiscard]] std::string dump(bool onlyNonNullOptProperties = true) const;

   private:
    void processNames(auto... names);
  };

  struct StyleSheet
  {
    template <typename... Args>
    StyleSheet(const std::string& name, Args&&... args)
        : name(name)
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
    MakeOptional<Style::Properties>::type styles;
    std::vector<StyleSheet> children;
  };

  StyleSheet styleSheet(const std::string& name, auto... p)
  {
    return StyleSheet { name, p... };
  }

  void Style::processNames(auto... names)
  {
    auto doNothing = [] {};
    for(const auto& name : { names... })
    {
      std::vector<const StyleSheet*> nestedSheets;

      for(auto* sheet : sheets)
      {
        for(const auto& nestedSheet : sheet->children)
        {
          if(nestedSheet.name == name)
          {
            if(std::count(sheets.begin(), sheets.end(), &nestedSheet) == 0)
              nestedSheets.push_back(&nestedSheet);

            std::apply([&](const auto&... a) { ((a.has_value() ? set(a.value()) : doNothing()), ...); }, nestedSheet.styles);
          }
        }
      }

      for(auto* sheet : nestedSheets)
        if(std::count(sheets.begin(), sheets.end(), sheet) == 0)
          sheets.push_back(sheet);
    }
  }

  Style Style::inherit(auto... names) const
  {
    auto doNothing = [] {};
    Style ret;
    ret.sheets = sheets;
    ret.copyFrom(*this, InheritableProperties {});
    ret.processNames(names...);
    return ret;
  }

  Style Style::add(auto... names) const
  {
    auto doNothing = [] {};
    Style ret = *this;
    ret.processNames(names...);
    return ret;
  }
}
