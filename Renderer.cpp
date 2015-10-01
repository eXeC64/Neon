#include "Renderer.hpp"

#include "Mesh.hpp"

namespace he
{

  Renderer::Renderer() :
    m_bIsInit(false),
    m_bufDiffuse(0), m_bufNormal(0), m_bufDepth(0), m_bufFBO(0)
  {};

  Renderer::~Renderer()
  {
  }

  bool Renderer::Init(int width, int height)
  {
    glClearColor(0,0,0,1);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    m_bIsInit = true;
    return true;
  }

  void Renderer::BeginFrame()
  {
    //Prepare for a new frame
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void Renderer::EndFrame()
  {
    //apply all our lights to produce the final output
  }

  void Renderer::SetProjectionMatrix(glm::mat4 matProjection)
  {
    m_matProjection = matProjection;
  }

  void Renderer::AddMesh(Mesh *pMesh, glm::mat4 matPosition)
  {
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, pMesh->m_vboVertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, pMesh->m_iStride, (void*)pMesh->m_iOffPos);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, pMesh->m_iStride, (void*)pMesh->m_iOffUV);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, pMesh->m_iStride, (void*)pMesh->m_iOffNormal);

    glDrawArrays(GL_TRIANGLES, 0, pMesh->m_iNumTris*3);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
  }

  void Renderer::AddLight(glm::vec3 pos, glm::vec3 rgb, double radius)
  {
    //add this light to the list to be rendered this frame
  }
}
