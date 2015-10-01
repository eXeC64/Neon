#pragma once

#include <glm/vec3.hpp>
#include "OpenGL.hpp"

namespace he
{
  class Mesh
  {
    friend class Renderer;
    friend class Loader;

  public:
    ~Mesh();

  private:
    Mesh();
    GLuint m_vboVertices; //VBO containing data
    int m_iNumTris; //Number of triangles total
    int m_iStride; //Number of bytes until next value of same type
    int m_iOffPos; //The offset to position data (-1 if not given)
    int m_iOffUV; //The offset to UV data (-1 if not given)
    int m_iOffNormal; //The offset to normal data (-1 if not given)
  };
}
