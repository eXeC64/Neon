#pragma once

#include "Mesh.hpp"

#include <string>

namespace he
{
  class Loader
  {
  public:
    static Mesh* LoadOBJ(const std::string &path);
  };
}
