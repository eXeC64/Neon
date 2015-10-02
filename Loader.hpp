#pragma once

#include <string>

namespace he
{
  class Mesh;
  class Texture;

  class Loader
  {
  public:
    static Mesh* LoadOBJ(const std::string &path);
    static Mesh* Plane();
    static Texture* LoadPNG(const std::string &path);
  };
}
