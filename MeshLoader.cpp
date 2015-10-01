#include "MeshLoader.hpp"

namespace he
{
  Mesh* MeshLoader::Dummy()
  {
    Mesh *pMesh = new Mesh();
    pMesh->m_iNumTris = 1;
    pMesh->m_iStride = 3 * sizeof(GLfloat);
    pMesh->m_iOffPos = 0 * sizeof(GLfloat);

    std::vector<GLfloat> data = {
      -1,-1,0,
       1,-1,0,
       0, 1,0,
    };

    glGenBuffers(1, &pMesh->m_vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, pMesh->m_vboVertices);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), &data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return pMesh;
  }
}
