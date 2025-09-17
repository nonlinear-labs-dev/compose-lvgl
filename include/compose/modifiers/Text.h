#pragma once
#include <string>
#include <concepts>

template <typename T>
concept tNumber = std::integral<T> || std::floating_point<T>;

template <typename T>
concept tWChar = std::is_same_v<T, wchar_t>;

namespace Compose
{
  struct Text
  {
    template <tNumber T>
    explicit Text(T i)
        : text { std::to_string(i) }
    {
    }

    template <typename T>
    explicit Text(T i)
        : text { i }
    {
    }

    template <typename... tArgs>
    explicit Text(tArgs... args)
        : text { std::format(args...) }
    {
    }

    Text() = default;
    bool operator==(const Text &) const = default;

    std::string text;
  };
}