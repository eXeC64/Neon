#include "StaticModel.hpp"
#include "StaticMesh.hpp"
#include "OpenGL.hpp"

namespace ne
{

  StaticModel::StaticModel()
  {
  }

  StaticModel::~StaticModel()
  {
    for(StaticMesh* mesh : m_meshes)
      delete mesh;
  }

}
