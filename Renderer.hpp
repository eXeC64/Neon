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

    void AddMesh(Mesh *pMesh, glm::mat4 matPosition); //adds a mesh to this frame
    void AddLight(glm::vec3 pos, glm::vec3 rgb, double radius); //adds a light to this frame

  private:
    GLuint LoadShader(const std::string &vsPath, const std::string &fsPath);

    bool m_bIsInit;
    glm::mat4 m_matProjection; //The camera's projection matrix
    GLuint m_shader;
  };
}
