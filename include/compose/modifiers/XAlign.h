#pragma once

enum class XAlignment : int
{
  LEFT,
  CENTER,
  RIGHT
};

namespace Compose
{
  struct XAlign
  {
    XAlignment it;
    static XAlign LEFT() { return {XAlignment::LEFT}; }
    static XAlign CENTER() { return {XAlignment::CENTER}; }
    static XAlign RIGHT() { return {XAlignment::RIGHT}; }
  };
} 