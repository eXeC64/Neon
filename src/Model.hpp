#pragma once

#include <glm/vec3.hpp>
#include "OpenGL.hpp"
#include <stdint.h>
#include <vector>

namespace ne
{
  class StaticMesh;
  class Material;

  class Model
  {
    friend class Renderer;
    friend class Loader;

  public:
    ~Model();

  private:
    Model();
  public:
    std::vector<StaticMesh*> m_meshes;
    std::vector<Material*> m_materials;
  };
}
