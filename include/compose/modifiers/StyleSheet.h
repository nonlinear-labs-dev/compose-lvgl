#pragma once
#include <glibmm/ustring.h>

namespace Compose
{
  struct StyleSheet
  {
    static StyleSheet CLASS(const char *t)
    {
      return StyleSheet(t);
    }

    Glib::ustring className;
  };
} 