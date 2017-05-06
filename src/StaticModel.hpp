#pragma once

#include <glm/vec3.hpp>
#include "OpenGL.hpp"
#include <stdint.h>
#include <vector>

namespace ne
{
  class StaticMesh;
  class Material;

  class StaticModel
  {
    friend class Renderer;
    friend class Loader;

  public:
    ~StaticModel();

  private:
    StaticModel();
  public:
    std::vector<StaticMesh*> m_meshes;
    std::vector<Material*> m_materials;
  };
}
