#pragma once

#include <string>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include "OpenGL.hpp"

namespace ne
{
  class StaticMesh;
  class AnimatedMesh;
  class Material;
  class Texture;

  struct StaticMeshInstance
  {
    StaticMeshInstance(StaticMesh* pMesh, Material* pMat, glm::mat4 position)
      : mesh(pMesh), mat(pMat), pos(position) {};
    StaticMesh* mesh;
    Material* mat;
    glm::mat4 pos;
  };

  struct AnimatedMeshInstance
  {
    AnimatedMeshInstance(AnimatedMesh* pMesh, Material* pMat, glm::mat4 position, const std::vector<glm::mat4> *boneTransforms)
      : mesh(pMesh), mat(pMat), pos(position), boneTransforms(boneTransforms) {};
    AnimatedMesh* mesh;
    Material* mat;
    glm::mat4 pos;
    const std::vector<glm::mat4> *boneTransforms;
  };

  struct PointLight
  {
    PointLight(glm::vec3 pos, glm::vec3 color, float brightness)
      : pos(pos), color(color), brightness(brightness) {};
    glm::vec3 pos;
    glm::vec3 color;
    float brightness;
  };

  struct DirectionalLight
  {
    DirectionalLight(glm::vec3 dir, glm::vec3 color, float brightness)
      : dir(dir), color(color), brightness(brightness) {};
    glm::vec3 dir;
    glm::vec3 color;
    float brightness;
  };

  struct SpotLight
  {
    SpotLight(glm::vec3 pos, glm::vec3 dir, float innerAngle, float outerAngle, glm::vec3 color, float brightness)
      : pos(pos)
      , dir(dir)
      , innerAngle(innerAngle)
      , outerAngle(outerAngle)
      , color(color)
      , brightness(brightness) {};
    glm::vec3 pos;
    glm::vec3 dir;
    float innerAngle;
    float outerAngle;
    glm::vec3 color;
    float brightness;
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
    void SetGamma(float gamma);
    void SetExposure(float exposure);

    //Add to current frame
    void AddStaticMesh(StaticMesh *pMesh, Material *pMat, glm::mat4 matPosition);
    void AddAnimatedMesh(AnimatedMesh *pMesh, Material *pMat, glm::mat4 matPosition, const std::vector<glm::mat4> *boneTransforms);
    void AddPointLight(const PointLight& light);
    void AddDirectionalLight(const DirectionalLight& light);
    void AddSpotLight(const SpotLight& light);

    //Add debug output
    void AddDebugCube(glm::mat4 position, glm::vec3 color);
    void AddDebugSphere(glm::mat4 position, glm::vec3 color);

    //Add to current time value
    void AddTime(double dt);

  private:
    GLuint LoadShader(const std::string &vsPath, const std::string &fsPath, const std::string &gsPath = "");

    void SetupGeometryPass();
    void SetupLightPass();
    void SetupDebugPass();
    void CompositeFrame();
    void DrawStaticMeshes();
    void DrawAnimatedMeshes();
    void DrawPointLights();
    void DrawDirectionalLights();
    void DrawSpotLights();
    void DrawSpotShadowMap(glm::mat4 matView);
    void DrawPointShadowMap(glm::vec3 position, double nearPlane, double farPlane);
    void DrawDebugMesh(const StaticMesh* mesh, const DebugInstance &instance);
    void UpdateProjectionMatrix();
    void ApplyGlobalIllumination();

    bool m_bIsInit;
    bool m_bIsMidFrame;
    int m_width;
    int m_height;
    int m_shadowMapSize;
    double m_curTime;
    glm::mat4 m_matProjection;
    glm::vec3 m_viewPos;
    float m_viewYaw;
    float m_viewTilt;
    float m_gamma;
    float m_exposure;
    GLuint m_shdStaticMesh;
    GLuint m_shdAnimatedMesh;
    GLuint m_shdPointLight;
    GLuint m_shdDirectionalLight;
    GLuint m_shdSpotLight;
    GLuint m_shdGlobalIllum;
    GLuint m_shdDebug;
    GLuint m_shdShadows;
    GLuint m_shdCubeShadows;
    GLuint m_shdAnimShadows;
    GLuint m_shdAnimCubeShadows;
    GLuint m_shdCompositor;
    GLuint m_texLambert;
    GLuint m_texNormal;
    GLuint m_texPBRMaps;
    GLuint m_texDepth;
    GLuint m_texComposite;
    GLuint m_FBO;
    GLuint m_shadowFBO;
    GLuint m_shadowCubeFBO;
    GLuint m_compositeFBO;
    GLuint m_texShadow;
    GLuint m_texShadowCube;
    StaticMesh* m_pPlane;
    StaticMesh* m_pCube;
    StaticMesh* m_pSphere;
    Texture *m_pDefaultLambert;
    Texture *m_pDefaultNormal;
    Texture *m_pDefaultMetallic;
    Texture *m_pDefaultRoughness;
    std::vector<StaticMeshInstance> m_staticMeshes;
    std::vector<AnimatedMeshInstance> m_animatedMeshes;
    std::vector<PointLight> m_pointLights;
    std::vector<DirectionalLight> m_directionalLights;
    std::vector<SpotLight> m_spotLights;
    std::vector<DebugInstance> m_debugCubes;
    std::vector<DebugInstance> m_debugSpheres;
    glm::vec3 m_globalIllumColor;
  };
}
