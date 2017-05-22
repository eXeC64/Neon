#pragma once

#include <string>
#include <unordered_map>

class aiAnimation;
class aiMesh;
class aiNode;
class aiScene;

namespace ne
{
  class AnimatedMesh;
  class AnimatedModel;
  class Animation;
  class Skeleton;
  class StaticMesh;
  class StaticModel;
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

    StaticModel* LoadStaticModel(const std::string &path);
    AnimatedModel* LoadAnimatedModel(const std::string &path);
    AnimatedMesh* LoadAnimatedMesh(const std::string& path);
    Animation* LoadAnimation(const std::string& path);
    Skeleton* LoadSkeleton(const std::string& path);
    Texture* LoadTexture(const std::string &path, enum TextureFormat format);
    StaticMesh* LoadBakedStaticMesh(const std::string& path);

  private:
    void ProcessModelNode(StaticModel* model, const aiScene* scene, const aiNode* node);
    StaticMesh* LoadStaticMesh(const aiMesh* mesh);


    std::unordered_map<std::string, Texture*> m_textures;
    std::unordered_map<std::string, StaticModel*> m_staticModels;
    std::unordered_map<std::string, AnimatedModel*> m_animatedModels;
  };
}
