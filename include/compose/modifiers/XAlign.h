#pragma once
#include <nltools/enums/EnumDecl.h>

ENUM(XAlignment, int, left, center, right);

namespace Compose
{
  struct XAlign
  {
    XAlignment it;
    static XAlign LEFT() { return {XAlignment::left}; }
    static XAlign CENTER() { return {XAlignment::center}; }
    static XAlign RIGHT() { return {XAlignment::right}; }
  };
} 