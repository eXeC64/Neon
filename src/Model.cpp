#include "Model.hpp"
#include "StaticMesh.hpp"
#include "OpenGL.hpp"

namespace ne
{

  Model::Model()
  {
  }

  Model::~Model()
  {
    for(StaticMesh* mesh : m_meshes)
      delete mesh;
  }

}
