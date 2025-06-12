#pragma once

namespace Compose
{
  struct Homogeneous
  {
    const bool it = true;

    static Homogeneous ON()
    {
      return Homogeneous(true);
    }
    static Homogeneous OFF()
    {
      return Homogeneous(false);
    }
  };

  struct RowHomogeneous : Homogeneous
  {
  };

  struct ColumnHomogeneous : Homogeneous
  {
  };
} 