#pragma once

namespace Compose
{
  struct Clickable
  {
    enum
    {
      Yes,
      No
    } it;

    Clickable(const Clickable &c) = default;

    explicit Clickable(bool b)
        : it { b ? Yes : No }
    {
    }

    explicit operator bool() const
    {
      return it == Yes;
    }

    bool operator==(const Clickable &) const = default;
  };
}