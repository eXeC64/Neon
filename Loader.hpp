#pragma once

#include "Mesh.hpp"

#include <string>

namespace he
{
  class Loader
  {
  public:
    static Mesh* LoadOBJ(const std::string &path);
    static Mesh* Plane();
    static Texture* LoadPNG(const std::string &path);
  };
}
