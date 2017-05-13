#include "AnimatedMesh.hpp"
#include "OpenGL.hpp"

namespace ne
{

  AnimatedMesh::AnimatedMesh() :
    m_vaoConfig(0),
    m_vboVertices(0),
    m_vboIndices(0),
    m_iNumTris(0),
    m_iNumIndices(0),
    m_iStride(0),
    m_iOffPos(-1), m_iOffNormal(-1), m_iOffUV(-1), m_iOffBoneWeights(-1), m_iOffBoneIds(-1)
  {
  }

  AnimatedMesh::~AnimatedMesh()
  {
    if(m_vboVertices)
      glDeleteBuffers(1, &m_vboVertices);
    if(m_vboIndices)
      glDeleteBuffers(1, &m_vboIndices);
    if(m_vaoConfig)
      glDeleteVertexArrays(1, &m_vaoConfig);
  }

}
