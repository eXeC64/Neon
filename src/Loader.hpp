#pragma once

#include <string>
#include <unordered_map>

class aiMesh;

namespace he
{
  class Model;
  class Mesh;
  class Texture;

  class Loader
  {
  public:
    Loader();
    ~Loader();

    static Mesh* GeneratePlane();
    Model* LoadModel(const std::string &path);
    Texture* LoadPNG(const std::string &path);

  private:
    Mesh* LoadMesh(const aiMesh* mesh);

    std::unordered_map<std::string, Texture*> m_textures;
    std::unordered_map<std::string, Model*> m_models;
  };
}
