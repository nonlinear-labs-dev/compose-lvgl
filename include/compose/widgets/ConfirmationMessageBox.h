#pragma once

#include <string>
#include <functional>

namespace Compose
{
  using ConfirmationCallback = std::function<void()>;
  void confirm(const std::string &title, const std::string &text, ConfirmationCallback callback);
}
