#pragma once

#include "Mesh.hpp"

#include <string>

namespace he
{
  class MeshLoader
  {
  public:
    static Mesh* LoadOBJ(const std::string &path);
    static Mesh* Dummy();
  };
}
