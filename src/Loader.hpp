#pragma once

#include <string>

class aiMesh;

namespace he
{
  class Model;
  class Mesh;
  class Texture;

  class Loader
  {
  public:
    static Mesh* LoadMesh(const aiMesh* mesh);
    static Model* LoadModel(const std::string &path);
    static Mesh* Plane();
    static Texture* LoadPNG(const std::string &path);
  };
}
