#pragma once

#include <string>
#include <vector>
#include <glm/mat4x4.hpp>
#include "OpenGL.hpp"

namespace he
{
  class Mesh;
  class Material;

  class Model
  {
  public:
    Model(Mesh* pMesh, Material* pMat, glm::mat4 position)
      : mesh(pMesh), mat(pMat), pos(position) {};
    Mesh* mesh;
    Material* mat;
    glm::mat4 pos;
  };

  class Renderer
  {
  public:
    Renderer();
    ~Renderer();

    bool Init(int width, int height); //Set up OpenGL resources

    void BeginFrame(); //Begin accepting geometry and lights for new frame
    void EndFrame(); //Draw current frame

    void SetProjectionMatrix(glm::mat4 matProjection);

    //Add to current frame
    void AddMesh(Mesh *pMesh, Material *pMat, glm::mat4 matPosition);
    void AddLight(glm::vec3 pos, glm::vec3 rgb, double radius);

  private:
    GLuint LoadShader(const std::string &vsPath, const std::string &fsPath);

    void SetupGeometryPass();
    void SetupLightPass();
    void DrawModel(const Model &model);

    bool m_bIsInit;
    bool m_bIsMidFrame;
    int m_width;
    int m_height;
    glm::mat4 m_matProjection; //The camera's projection matrix
    GLuint m_shdMesh;
    GLuint m_shdLight;
    GLuint m_texDiffuse;
    GLuint m_texNormal;
    GLuint m_texDepth;
    GLuint m_FBO;
    Mesh* m_pPlane;
    std::vector<Model> m_models;
  };
}
