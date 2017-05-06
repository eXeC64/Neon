#pragma once

#include <string>
#include <unordered_map>

class aiMesh;
class aiNode;
class aiScene;

namespace ne
{
  class Model;
  class StaticMesh;
  class Texture;

  enum class TextureFormat
  {
    Color,  //Already gamma corrected - needs linearizing
    Normal, //In linear space, represents a vector such as a normal
    Map     //Greyscale, used for roughness maps, etc.
  };

  class Loader
  {
  public:
    Loader();
    ~Loader();

    static StaticMesh* GeneratePlane();
    static StaticMesh* GenerateCube();
    static StaticMesh* GenerateSphere();
    static Texture* GenerateBlankNormal();
    static Texture* GenerateBlankMap(unsigned char value);
    static Texture* GeneratePurpleCheques();

    Model* LoadModel(const std::string &path);
    Texture* LoadTexture(const std::string &path, enum TextureFormat format);

  private:
    void ProcessModelNode(Model* model, const aiScene* scene, const aiNode* node);
    StaticMesh* LoadStaticMesh(const aiMesh* mesh);

    std::unordered_map<std::string, Texture*> m_textures;
    std::unordered_map<std::string, Model*> m_models;
  };
}
