#pragma once

#include <string>
#include <glm/mat4x4.hpp>
#include "OpenGL.hpp"

namespace he
{
  class Mesh;
  class Material;

  class Renderer
  {
  public:
    Renderer();
    ~Renderer();

    bool Init(int width, int height); //Set up OpenGL resources

    void BeginFrame(); //Clears buffers, prepares for drawing
    void EndFrame(); //applies all the lights and produces final output

    void SetProjectionMatrix(glm::mat4 matProjection);

    void AddMesh(Mesh *pMesh, Material *pMat, glm::mat4 matPosition);
    void AddLight(glm::vec3 pos, glm::vec3 rgb, double radius);

  private:
    GLuint LoadShader(const std::string &vsPath, const std::string &fsPath);

    bool m_bIsInit;
    int m_width;
    int m_height;
    glm::mat4 m_matProjection; //The camera's projection matrix
    GLuint m_shdMesh;
    GLuint m_shdLight;
    GLuint m_texDiffuse;
    GLuint m_texNormal;
    GLuint m_texWorldPos;
    GLuint m_texDepth;
    GLuint m_FBO;
    Mesh* m_pPlane;
  };
}
