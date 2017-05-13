#pragma once

#include <glm/vec3.hpp>
#include "OpenGL.hpp"
#include <stdint.h>
#include <vector>

#include "Bone.hpp"

namespace ne
{
  class AnimatedMesh
  {
    friend class Renderer;
    friend class Loader;

  public:
    ~AnimatedMesh();

  private:
    AnimatedMesh();
    GLuint m_vaoConfig; //Configuration vao
    GLuint m_vboVertices; //VBO containing vertices
    GLuint m_vboIndices; //VBO containing indices
    int m_iNumTris; //Number of triangles total
    int m_iNumIndices; //Number of indices in the buffer
    int m_iStride; //Number of bytes until next value of same type
    uintptr_t m_iOffPos; //The offset to position data
    uintptr_t m_iOffNormal; //The offset to normal data
    uintptr_t m_iOffUV; //The offset to UV data
    uintptr_t m_iOffBoneWeights; //The offset to bone weights
    uintptr_t m_iOffBoneIds; //The offset to bone ids
  };
}
