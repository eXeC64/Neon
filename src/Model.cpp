#include "Model.hpp"
#include "Mesh.hpp"
#include "OpenGL.hpp"

namespace he
{

  Model::Model()
  {
  }

  Model::~Model()
  {
    for(Mesh* mesh : m_meshes)
      delete mesh;
  }

}
