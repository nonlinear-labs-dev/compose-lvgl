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
    using Properties = std::tuple<BackgroundColor, PrimaryColor, Font, TextAlign, FlexAlign, Width, Height, Margin, MarginLeft, MarginRight, MarginTop, MarginBottom, Padding,
                                  Border, BorderSides, RoundedCorner, Scrollable>;
    MakeOptional<Properties>::type properties;
    std::vector<const StyleSheet*> sheets;

    template <typename P> void set(const P& p)
    {
      std::get<std::optional<P>>(properties) = p;
    }

    Style applyClasses(auto... names) const;
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

  Style Style::applyClasses(auto... names) const
  {
    auto doNothing = [] {};
    Style ret = *this;

    for(const auto& name : { names... })
    {
      for(auto* sheet : sheets)
      {
        for(const auto& nestedSheet : sheet->children)
        {
          if(std::count(ret.sheets.begin(), ret.sheets.end(), &nestedSheet) == 0)
            ret.sheets.push_back(&nestedSheet);

          if(nestedSheet.name == name)
          {
            std::apply([&](const auto&... a) { ((a.has_value() ? ret.set(a.value()) : doNothing()), ...); }, nestedSheet.styles);
          }
        }
      }
    }
    return ret;
  }
}