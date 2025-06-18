#pragma once

enum class FontWeightEnum
{
  WEIGHT_THIN = 100,
  WEIGHT_ULTRALIGHT = 200,
  WEIGHT_LIGHT = 300,
  WEIGHT_SEMILIGHT = 350,
  WEIGHT_BOOK = 380,
  WEIGHT_NORMAL = 400,
  WEIGHT_MEDIUM = 500,
  WEIGHT_SEMIBOLD = 600,
  WEIGHT_BOLD = 700,
  WEIGHT_ULTRABOLD = 800,
  WEIGHT_HEAVY = 900,
  WEIGHT_ULTRAHEAVY = 1000
};

namespace Compose
{
  struct FontWeight
  {
    FontWeightEnum it;
  };
}