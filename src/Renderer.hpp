#pragma once

#include <string>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include "OpenGL.hpp"

namespace he
{
  class Mesh;
  class Material;
  class Texture;

  struct MeshInstance
  {
    MeshInstance(Mesh* pMesh, Material* pMat, glm::mat4 position)
      : mesh(pMesh), mat(pMat), pos(position) {};
    Mesh* mesh;
    Material* mat;
    glm::mat4 pos;
  };

  struct LightInstance
  {
    LightInstance(glm::vec3 pos, glm::vec3 color)
      : pos(pos), color(color) {};
    glm::vec3 pos;
    glm::vec3 color;
  };

  struct DebugInstance
  {
    DebugInstance(glm::mat4 pos, glm::vec3 color)
      : pos(pos), color(color) {};
    glm::mat4 pos;
    glm::vec3 color;
  };

  class Renderer
  {
  public:
    Renderer();
    ~Renderer();

    bool Init(int width, int height); //Set up OpenGL resources

    void BeginFrame(); //Begin accepting geometry and lights for new frame
    void EndFrame(); //Draw current frame

    void SetViewPosition(glm::vec3 pos, float yaw, float tilt);
    void SetGlobalIllumination(glm::vec3 color);

    //Add to current frame
    void AddMesh(Mesh *pMesh, Material *pMat, glm::mat4 matPosition);
    void AddLight(glm::vec3 pos, glm::vec3 color);

    //Add debug output
    void AddDebugCube(glm::mat4 position, glm::vec3 color);
    void AddDebugSphere(glm::mat4 position, glm::vec3 color);

    //Add to current time value
    void AddTime(double dt);

  private:
    GLuint LoadShader(const std::string &vsPath, const std::string &fsPath);

    void SetupGeometryPass();
    void SetupLightPass();
    void SetupDebugPass();
    void DrawMeshInstance(const MeshInstance &model);
    void DrawLightInstance(const LightInstance &light);
    void DrawDebugMesh(const Mesh* mesh, const DebugInstance &instance);
    void UpdateProjectionMatrix();
    void ApplyGlobalIllumination();

    bool m_bIsInit;
    bool m_bIsMidFrame;
    int m_width;
    int m_height;
    double m_curTime;
    glm::mat4 m_matProjection;
    glm::vec3 m_viewPos;
    float m_viewYaw;
    float m_viewTilt;
    GLuint m_shdMesh;
    GLuint m_shdLight;
    GLuint m_shdGlobalIllum;
    GLuint m_shdDebug;
    GLuint m_texLambert;
    GLuint m_texNormal;
    GLuint m_texPBRMaps;
    GLuint m_texDepth;
    GLuint m_FBO;
    Mesh* m_pPlane;
    Mesh* m_pCube;
    Mesh* m_pSphere;
    Texture *m_pDefaultLambert;
    Texture *m_pDefaultNormal;
    Texture *m_pDefaultMetallic;
    Texture *m_pDefaultRoughness;
    std::vector<MeshInstance> m_models;
    std::vector<LightInstance> m_lights;
    std::vector<DebugInstance> m_debugCubes;
    std::vector<DebugInstance> m_debugSpheres;
    glm::vec3 m_globalIllumColor;
  };
}
