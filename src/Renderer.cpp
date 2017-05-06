#include "Renderer.hpp"

#include "Mesh.hpp"
#include "Material.hpp"
#include "Texture.hpp"
#include "Loader.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <glm/gtc/matrix_transform.hpp>

namespace
{
  GLuint GenerateBuffer(GLint format, GLint component, GLint attachment, GLsizei width, GLsizei height)
  {
    GLuint buf;
    glGenTextures(1, &buf);
    glBindTexture(GL_TEXTURE_2D, buf);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, component, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, GL_TEXTURE_2D, buf, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    return buf;
  }
}

namespace ne
{

  Renderer::Renderer() :
    m_bIsInit(false),
    m_bIsMidFrame(false),
    m_width(0), m_height(0),
    m_shadowMapSize(1024),
    m_curTime(0),
    m_viewYaw(0),
    m_viewTilt(0),
    m_gamma(2.2),
    m_shdMesh(0),
    m_shdPointLight(0),
    m_shdDirectionalLight(0),
    m_shdSpotLight(0),
    m_shdGlobalIllum(0),
    m_shdDebug(0),
    m_shdShadows(0),
    m_shdCubeShadows(0),
    m_shdCompositor(0),
    m_texLambert(0),
    m_texNormal(0),
    m_texPBRMaps(0),
    m_texDepth(0),
    m_texComposite(0),
    m_FBO(0),
    m_shadowFBO(0),
    m_shadowCubeFBO(0),
    m_compositeFBO(0),
    m_texShadow(0),
    m_texShadowCube(0),
    m_pPlane(nullptr),
    m_pCube(nullptr),
    m_pSphere(nullptr),
    m_pDefaultLambert(nullptr),
    m_pDefaultNormal(nullptr),
    m_pDefaultMetallic(nullptr),
    m_pDefaultRoughness(nullptr)
  {};

  Renderer::~Renderer()
  {
    if(m_shdMesh)
      glDeleteProgram(m_shdMesh);
    if(m_shdPointLight)
      glDeleteProgram(m_shdPointLight);
    if(m_shdDirectionalLight)
      glDeleteProgram(m_shdDirectionalLight);
    if(m_shdSpotLight)
      glDeleteProgram(m_shdSpotLight);
    if(m_shdGlobalIllum)
      glDeleteProgram(m_shdGlobalIllum);
    if(m_shdDebug)
      glDeleteProgram(m_shdDebug);
    if(m_shdShadows)
      glDeleteProgram(m_shdShadows);
    if(m_shdCubeShadows)
      glDeleteProgram(m_shdCubeShadows);
    if(m_shdCompositor)
      glDeleteProgram(m_shdCompositor);
    if(m_texLambert)
      glDeleteTextures(1, &m_texLambert);
    if(m_texNormal)
      glDeleteTextures(1, &m_texNormal);
    if(m_texPBRMaps)
      glDeleteTextures(1, &m_texPBRMaps);
    if(m_texDepth)
      glDeleteTextures(1, &m_texDepth);
    if(m_texComposite)
      glDeleteTextures(1, &m_texComposite);
    if(m_FBO)
      glDeleteFramebuffers(1, &m_FBO);
    if(m_shadowFBO)
      glDeleteFramebuffers(1, &m_shadowFBO);
    if(m_shadowCubeFBO)
      glDeleteFramebuffers(1, &m_shadowCubeFBO);
    if(m_compositeFBO)
      glDeleteFramebuffers(1, &m_compositeFBO);
    if(m_texShadow)
      glDeleteTextures(1, &m_texShadow);
    if(m_texShadowCube)
      glDeleteTextures(1, &m_texShadowCube);
    if(m_pPlane)
      delete m_pPlane;
    if(m_pCube)
      delete m_pCube;
    if(m_pSphere)
      delete m_pSphere;
    if(m_pDefaultLambert)
      delete m_pDefaultLambert;
    if(m_pDefaultNormal)
      delete m_pDefaultNormal;
    if(m_pDefaultMetallic)
      delete m_pDefaultMetallic;
    if(m_pDefaultRoughness)
      delete m_pDefaultRoughness;
  }

  bool Renderer::Init(int width, int height)
  {
    m_width = width;
    m_height = height;

    //Construct a frame buffer
    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);

    m_texLambert = GenerateBuffer(GL_RGB8, GL_RGB, GL_COLOR_ATTACHMENT0, m_width, m_height);
    m_texNormal = GenerateBuffer(GL_RGB16F, GL_RGB, GL_COLOR_ATTACHMENT1, m_width, m_height);
    m_texPBRMaps = GenerateBuffer(GL_RG16F, GL_RG, GL_COLOR_ATTACHMENT2, m_width, m_height);
    m_texDepth = GenerateBuffer(GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_DEPTH_ATTACHMENT, m_width, m_height);

    GLenum drawBuffers[] = {
      GL_COLOR_ATTACHMENT0,
      GL_COLOR_ATTACHMENT1,
      GL_COLOR_ATTACHMENT2
    };
    glDrawBuffers(sizeof drawBuffers / sizeof drawBuffers[0], drawBuffers);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      return false;


    //Setup framebuffer for compositing
    glGenFramebuffers(1, &m_compositeFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_compositeFBO);

    m_texComposite = GenerateBuffer(GL_RGB16F, GL_RGB, GL_COLOR_ATTACHMENT0, m_width, m_height);
    GLenum compositeBuffers[] = {
      GL_COLOR_ATTACHMENT0,
    };
    glDrawBuffers(sizeof compositeBuffers / sizeof compositeBuffers[0], compositeBuffers);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      return false;


    //Return to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //Setup frame buffer for shadows
    glGenTextures(1, &m_texShadow);
    glBindTexture(GL_TEXTURE_2D, m_texShadow);
    glTexImage2D(
        GL_TEXTURE_2D,
        0, GL_DEPTH_COMPONENT,
        m_shadowMapSize,
        m_shadowMapSize,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[] = { 0.0, 0.0, 0.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glGenFramebuffers(1, &m_shadowFBO);

    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_texShadow, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    //Return to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //Setup frame buffer for shadows
    glGenTextures(1, &m_texShadowCube);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_texShadowCube);
    for(int i = 0; i < 6; ++i)
    {
      glTexImage2D(
          GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
          0, GL_DEPTH_COMPONENT,
          m_shadowMapSize,
          m_shadowMapSize,
          0,
          GL_DEPTH_COMPONENT,
          GL_FLOAT,
          NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, borderColor);

    glGenFramebuffers(1, &m_shadowCubeFBO);

    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowCubeFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_texShadowCube, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    //Return to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);


    m_shdMesh = LoadShader("shaders/mesh_vert.glsl", "shaders/mesh_frag.glsl");
    if(!m_shdMesh)
      return false;

    m_shdPointLight = LoadShader("shaders/light_vert.glsl", "shaders/pointlight_frag.glsl");
    if(!m_shdPointLight)
      return false;

    m_shdDirectionalLight = LoadShader("shaders/light_vert.glsl", "shaders/dirlight_frag.glsl");
    if(!m_shdDirectionalLight)
      return false;

    m_shdSpotLight = LoadShader("shaders/spotlight_vert.glsl", "shaders/spotlight_frag.glsl");
    if(!m_shdSpotLight)
      return false;

    m_shdGlobalIllum = LoadShader("shaders/globalillum_vert.glsl", "shaders/globalillum_frag.glsl");
    if(!m_shdGlobalIllum)
      return false;

    m_shdDebug = LoadShader("shaders/debug_vert.glsl", "shaders/debug_frag.glsl");
    if(!m_shdDebug)
      return false;

    m_shdShadows = LoadShader("shaders/shadows_vert.glsl", "shaders/shadows_frag.glsl");
    if(!m_shdShadows)
      return false;

    m_shdCubeShadows = LoadShader("shaders/cubeshadows_vert.glsl", "shaders/cubeshadows_frag.glsl", "shaders/cubeshadows_geom.glsl");
    if(!m_shdCubeShadows)
      return false;

    m_shdCompositor = LoadShader("shaders/compositor_vert.glsl", "shaders/compositor_frag.glsl");
    if(!m_shdCompositor)
      return false;

    m_pPlane = Loader::GeneratePlane();
    if(!m_pPlane)
      return false;

    m_pCube = Loader::GenerateCube();
    if(!m_pCube)
      return false;

    m_pSphere = Loader::GenerateSphere();
    if(!m_pSphere)
      return false;

    m_pDefaultLambert = Loader::GeneratePurpleCheques();
    if(!m_pDefaultLambert)
      return false;

    m_pDefaultNormal = Loader::GenerateBlankNormal();
    if(!m_pDefaultNormal)
      return false;

    m_pDefaultMetallic = Loader::GenerateBlankMap(0);
    if(!m_pDefaultMetallic)
      return false;

    m_pDefaultRoughness = Loader::GenerateBlankMap(255);
    if(!m_pDefaultRoughness)
      return false;

    m_bIsInit = true;
    return true;
  }

  void Renderer::BeginFrame()
  {
    m_bIsMidFrame = true;
    //Clear out existing lights and geometry
    m_models.clear();
    m_pointLights.clear();
    m_directionalLights.clear();
    m_spotLights.clear();
    m_debugCubes.clear();
    m_debugSpheres.clear();
  }

  void Renderer::EndFrame()
  {
    //Prepare for geometry pass
    SetupGeometryPass();

    //Draw the geometry into the g buffers
    DrawStaticMeshes();

    //Prepare for lighting pass
    SetupLightPass();

    //Perform global illumination
    ApplyGlobalIllumination();

    //Apply all our lights
    DrawPointLights();
    DrawDirectionalLights();
    DrawSpotLights();

    CompositeFrame();

    //TODO in future: final pass for transparent/translucent objects

    SetupDebugPass();
    for (auto& cube : m_debugCubes)
      DrawDebugMesh(m_pCube, cube);
    for (auto& sphere : m_debugSpheres)
      DrawDebugMesh(m_pSphere, sphere);

    m_bIsMidFrame = false;
  }

  void Renderer::SetViewPosition(glm::vec3 pos, float yaw, float tilt)
  {
    if(m_bIsMidFrame)
      return;

    m_viewPos = pos;
    m_viewYaw = yaw;
    m_viewTilt = tilt;

    UpdateProjectionMatrix();
  }

  void Renderer::UpdateProjectionMatrix()
  {
    glm::mat4 proj = glm::perspective(glm::radians(65.0), 16.0/9.0, 0.1, 100.0);
    glm::mat4 rot =
      glm::rotate(glm::mat4(1.0), m_viewTilt, glm::vec3(1,0,0)) *
      glm::rotate(glm::mat4(1.0), m_viewYaw, glm::vec3(0,1,0));
    glm::mat4 tran = glm::translate(glm::mat4(1.0), -m_viewPos);
    m_matProjection = proj * rot * tran;
  }

  void Renderer::AddMesh(Mesh *pMesh, Material *pMat, glm::mat4 matPosition)
  {
    if(!pMesh || !pMat || !m_bIsMidFrame)
      return;

    m_models.push_back(MeshInstance(pMesh, pMat, matPosition));
  }

  void Renderer::DrawStaticMeshes()
  {
    glUseProgram(m_shdMesh);
    glUniformMatrix4fv(glGetUniformLocation(m_shdMesh, "matView"), 1, GL_FALSE, &m_matProjection[0][0]);

    glUniform1i(glGetUniformLocation(m_shdMesh, "sampLambert"), 0);
    glUniform1i(glGetUniformLocation(m_shdMesh, "sampNormal"), 1);
    glUniform1i(glGetUniformLocation(m_shdMesh, "sampMetallic"), 2);
    glUniform1i(glGetUniformLocation(m_shdMesh, "sampRoughness"), 3);

    GLint matPosLoc = glGetUniformLocation(m_shdMesh, "matPos");
    for(auto& model : m_models)
    {
      glUniformMatrix4fv(matPosLoc, 1, GL_FALSE, &model.pos[0][0]);

      Texture *pLambert = model.mat->m_pLambert;
      if(!pLambert)
        pLambert = m_pDefaultLambert;

      Texture *pNormal = model.mat->m_pNormal;
      if(!pNormal)
        pNormal = m_pDefaultNormal;

      Texture *pMetallic = model.mat->m_pMetallic;
      if(!pMetallic)
        pMetallic = m_pDefaultMetallic;

      Texture *pRoughness = model.mat->m_pRoughness;
      if(!pRoughness)
        pRoughness = m_pDefaultRoughness;

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, pLambert->m_glTexture);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, pNormal->m_glTexture);
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, pMetallic->m_glTexture);
      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, pRoughness->m_glTexture);

      glBindVertexArray(model.mesh->m_vaoConfig);

      if(model.mesh->m_iNumIndices > 0)
      {
        glDrawElements(GL_TRIANGLES, model.mesh->m_iNumIndices, GL_UNSIGNED_INT, 0);
      }
      else
      {
        glDrawArrays(GL_TRIANGLES, 0, model.mesh->m_iNumTris*3);
      }
    }

    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
  }

  void Renderer::DrawPointLights()
  {
    const GLint matViewLoc         = glGetUniformLocation(m_shdPointLight, "matView");
    const GLint viewPosLoc         = glGetUniformLocation(m_shdPointLight, "viewPos");
    const GLint screenSizeLoc      = glGetUniformLocation(m_shdPointLight, "screenSize");
    const GLint sampLambertLoc     = glGetUniformLocation(m_shdPointLight, "sampLambert");
    const GLint sampNormalLoc      = glGetUniformLocation(m_shdPointLight, "sampNormal");
    const GLint sampPBRMapsLoc     = glGetUniformLocation(m_shdPointLight, "sampPBRMaps");
    const GLint sampDepthLoc       = glGetUniformLocation(m_shdPointLight, "sampDepth");
    const GLint sampShadowLoc      = glGetUniformLocation(m_shdPointLight, "sampShadow");
    const GLint lightPosLoc        = glGetUniformLocation(m_shdPointLight, "lightPos");
    const GLint lightColorLoc      = glGetUniformLocation(m_shdPointLight, "lightColor");
    const GLint lightBrightnessLoc = glGetUniformLocation(m_shdPointLight, "lightBrightness");
    const GLint farPlaneLoc        = glGetUniformLocation(m_shdPointLight, "farPlane");


    for(auto& light : m_pointLights)
    {
      // First render shadow map
      const double nearPlane = 0.1, farPlane = 30.0;
      DrawPointShadowMap(light.pos, nearPlane, farPlane);
      glBindFramebuffer(GL_FRAMEBUFFER, m_compositeFBO);

      // Now render lighting shader
      glUseProgram(m_shdPointLight);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, m_texLambert);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, m_texNormal);

      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, m_texPBRMaps);

      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, m_texDepth);

      glActiveTexture(GL_TEXTURE4);
      glBindTexture(GL_TEXTURE_CUBE_MAP, m_texShadowCube);

      glUniformMatrix4fv(matViewLoc, 1, GL_FALSE, &m_matProjection[0][0]);
      glUniform3f(viewPosLoc, m_viewPos.x, m_viewPos.y, m_viewPos.z);
      glUniform2f(screenSizeLoc, (float)m_width, (float)m_height);
      glUniform1i(sampLambertLoc, 0);
      glUniform1i(sampNormalLoc,  1);
      glUniform1i(sampPBRMapsLoc, 2);
      glUniform1i(sampDepthLoc,   3);
      glUniform1i(sampShadowLoc,  4);
      glUniform3f(lightPosLoc, light.pos.x, light.pos.y, light.pos.z);
      glUniform3f(lightColorLoc, light.color.x, light.color.y, light.color.z);
      glUniform1f(lightBrightnessLoc, light.brightness);
      glUniform1f(farPlaneLoc, (float)farPlane);

      glDepthFunc(GL_ALWAYS);
      glBindVertexArray(m_pPlane->m_vaoConfig);
      glDrawArrays(GL_TRIANGLES, 0, m_pPlane->m_iNumTris*3);
      glBindVertexArray(0);
    }
  }

  void Renderer::DrawDirectionalLights()
  {
    glUseProgram(m_shdDirectionalLight);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texLambert);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_texNormal);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_texPBRMaps);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_texDepth);

    glUniform2f(glGetUniformLocation(m_shdDirectionalLight, "screenSize"), (float)m_width, (float)m_height);
    glUniform1i(glGetUniformLocation(m_shdDirectionalLight, "sampLambert"), 0);
    glUniform1i(glGetUniformLocation(m_shdDirectionalLight, "sampNormal"), 1);
    glUniform1i(glGetUniformLocation(m_shdDirectionalLight, "sampPBRMaps"), 2);
    glUniform1i(glGetUniformLocation(m_shdDirectionalLight, "sampDepth"), 3);

    const GLint lightDirLoc = glGetUniformLocation(m_shdDirectionalLight, "lightDir");
    const GLint lightColorLoc = glGetUniformLocation(m_shdDirectionalLight, "lightColor");
    const GLint lightBrightnessLoc = glGetUniformLocation(m_shdDirectionalLight, "lightBrightness");

    glBindVertexArray(m_pPlane->m_vaoConfig);
    for(auto& light : m_directionalLights)
    {
      glUniform3f(lightDirLoc, light.dir.x, light.dir.y, light.dir.z);
      glUniform3f(lightColorLoc, light.color.x, light.color.y, light.color.z);
      glUniform1f(lightBrightnessLoc, light.brightness);
      glDrawArrays(GL_TRIANGLES, 0, m_pPlane->m_iNumTris*3);
    }
    glBindVertexArray(0);
  }

  void Renderer::DrawSpotLights()
  {
    const GLint matViewLoc         = glGetUniformLocation(m_shdSpotLight, "matView");
    const GLint matLightLoc        = glGetUniformLocation(m_shdSpotLight, "matLight");
    const GLint screenSizeLoc      = glGetUniformLocation(m_shdSpotLight, "screenSize");
    const GLint sampLambertLoc     = glGetUniformLocation(m_shdSpotLight, "sampLambert");
    const GLint sampNormalLoc      = glGetUniformLocation(m_shdSpotLight, "sampNormal");
    const GLint sampPBRMapsLoc     = glGetUniformLocation(m_shdSpotLight, "sampPBRMaps");
    const GLint sampDepthLoc       = glGetUniformLocation(m_shdSpotLight, "sampDepth");
    const GLint sampShadowLoc      = glGetUniformLocation(m_shdSpotLight, "sampShadow");
    const GLint lightPosLoc        = glGetUniformLocation(m_shdSpotLight, "lightPos");
    const GLint lightDirLoc        = glGetUniformLocation(m_shdSpotLight, "lightDir");
    const GLint innerAngleLoc      = glGetUniformLocation(m_shdSpotLight, "innerAngle");
    const GLint outerAngleLoc      = glGetUniformLocation(m_shdSpotLight, "outerAngle");
    const GLint lightColorLoc      = glGetUniformLocation(m_shdSpotLight, "lightColor");
    const GLint lightBrightnessLoc = glGetUniformLocation(m_shdSpotLight, "lightBrightness");
    const GLint nearPlaneLoc       = glGetUniformLocation(m_shdSpotLight, "nearPlane");
    const GLint farPlaneLoc        = glGetUniformLocation(m_shdSpotLight, "farPlane");

    for(auto& light : m_spotLights)
    {
      //First render shadow map
      const double nearPlane = 0.1, farPlane = 30.0;
      const glm::mat4 lightProj = glm::perspective(light.outerAngle * 2.0, 1.0, nearPlane, farPlane);
      const glm::mat4 lightView = glm::lookAt(light.pos, light.pos + light.dir, glm::vec3(0,1,0));
      const glm::mat4 lightSpace = lightProj * lightView;

      DrawSpotShadowMap(lightSpace);
      glBindFramebuffer(GL_FRAMEBUFFER, m_compositeFBO);

      // Now render lighting shader
      glUseProgram(m_shdSpotLight);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, m_texLambert);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, m_texNormal);

      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, m_texPBRMaps);

      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, m_texDepth);

      glActiveTexture(GL_TEXTURE4);
      glBindTexture(GL_TEXTURE_2D, m_texShadow);

      glUniformMatrix4fv(matViewLoc, 1, GL_FALSE, &m_matProjection[0][0]);
      glUniformMatrix4fv(matLightLoc, 1, GL_FALSE, &lightSpace[0][0]);
      glUniform2f(screenSizeLoc, (float)m_width, (float)m_height);
      glUniform1i(sampLambertLoc, 0);
      glUniform1i(sampNormalLoc,  1);
      glUniform1i(sampPBRMapsLoc, 2);
      glUniform1i(sampDepthLoc,   3);
      glUniform1i(sampShadowLoc,  4);
      glUniform3f(lightPosLoc, light.pos.x, light.pos.y, light.pos.z);
      glUniform3f(lightDirLoc, light.dir.x, light.dir.y, light.dir.z);
      glUniform1f(innerAngleLoc, glm::cos(light.innerAngle));
      glUniform1f(outerAngleLoc, glm::cos(light.outerAngle));
      glUniform3f(lightColorLoc, light.color.x, light.color.y, light.color.z);
      glUniform1f(lightBrightnessLoc, light.brightness);
      glUniform1f(nearPlaneLoc, nearPlane);
      glUniform1f(farPlaneLoc, farPlane);

      glDepthFunc(GL_ALWAYS);
      glBindVertexArray(m_pPlane->m_vaoConfig);
      glDrawArrays(GL_TRIANGLES, 0, m_pPlane->m_iNumTris*3);
      glBindVertexArray(0);
    }
  }

  void Renderer::DrawSpotShadowMap(glm::mat4 lightProj)
  {
    glViewport(0, 0, m_shadowMapSize, m_shadowMapSize);
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glDepthFunc(GL_LESS);
    glUseProgram(m_shdShadows);
    glUniformMatrix4fv(glGetUniformLocation(m_shdShadows, "matLight"), 1, GL_FALSE, &lightProj[0][0]);
    const GLint matPosLoc = glGetUniformLocation(m_shdShadows, "matPos");

    for(auto& model : m_models)
    {
      glUniformMatrix4fv(matPosLoc, 1, GL_FALSE, &model.pos[0][0]);

      glBindVertexArray(model.mesh->m_vaoConfig);

      if(model.mesh->m_iNumIndices > 0)
      {
        glDrawElements(GL_TRIANGLES, model.mesh->m_iNumIndices, GL_UNSIGNED_INT, 0);
      }
      else
      {
        glDrawArrays(GL_TRIANGLES, 0, model.mesh->m_iNumTris*3);
      }

      glBindVertexArray(0);
    }

    glViewport(0, 0, m_width, m_height);
  }

  void Renderer::DrawPointShadowMap(glm::vec3 position, double nearPlane, double farPlane)
  {
    glViewport(0, 0, m_shadowMapSize, m_shadowMapSize);
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowCubeFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glDepthFunc(GL_LESS);
    glUseProgram(m_shdCubeShadows);

    glm::mat4 lightProj = glm::perspective(glm::radians(90.0), 1.0, nearPlane, farPlane);
    glm::mat4 lightTransforms[6] = {
      lightProj * glm::lookAt(position, position + glm::vec3( 1,0,0), glm::vec3(0,-1,0)),
      lightProj * glm::lookAt(position, position + glm::vec3(-1,0,0), glm::vec3(0,-1,0)),
      lightProj * glm::lookAt(position, position + glm::vec3(0, 1,0), glm::vec3(0,0, 1)),
      lightProj * glm::lookAt(position, position + glm::vec3(0,-1,0), glm::vec3(0,0,-1)),
      lightProj * glm::lookAt(position, position + glm::vec3(0,0, 1), glm::vec3(0,-1,0)),
      lightProj * glm::lookAt(position, position + glm::vec3(0,0,-1), glm::vec3(0,-1,0))
    };

    for(int i = 0; i < 6; ++i)
    {
      std::string name = "matLight[" + std::to_string(i) + "]";
      glUniformMatrix4fv(glGetUniformLocation(m_shdCubeShadows, name.c_str()), 1, GL_FALSE, &lightTransforms[i][0][0]);
    }

    glUniform3f(glGetUniformLocation(m_shdCubeShadows, "lightPos"), position.x, position.y, position.z);
    glUniform1f(glGetUniformLocation(m_shdCubeShadows, "farPlane"), (float)farPlane);
    const GLint matPosLoc = glGetUniformLocation(m_shdCubeShadows, "matPos");

    for(auto& model : m_models)
    {
      glUniformMatrix4fv(matPosLoc, 1, GL_FALSE, &model.pos[0][0]);

      glBindVertexArray(model.mesh->m_vaoConfig);

      if(model.mesh->m_iNumIndices > 0)
      {
        glDrawElements(GL_TRIANGLES, model.mesh->m_iNumIndices, GL_UNSIGNED_INT, 0);
      }
      else
      {
        glDrawArrays(GL_TRIANGLES, 0, model.mesh->m_iNumTris*3);
      }

      glBindVertexArray(0);
    }

    glViewport(0, 0, m_width, m_height);
  }

  void Renderer::DrawDebugMesh(const Mesh* mesh, const DebugInstance &instance)
  {
    glUseProgram(m_shdDebug);
    glUniformMatrix4fv(glGetUniformLocation(m_shdDebug, "matView"), 1, GL_FALSE, &m_matProjection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_shdDebug, "matPos"), 1, GL_FALSE, &instance.pos[0][0]);
    glUniform3f(glGetUniformLocation(m_shdDebug, "color"), instance.color.x, instance.color.y, instance.color.z);

    glBindVertexArray(mesh->m_vaoConfig);
    if(mesh->m_iNumIndices > 0) {
      glDrawElements(GL_TRIANGLES, mesh->m_iNumIndices, GL_UNSIGNED_INT, 0);
    } else {
      glDrawArrays(GL_TRIANGLES, 0, mesh->m_iNumTris*3);
    }
    glBindVertexArray(0);
  }

  void Renderer::AddPointLight(const PointLight& light)
  {
    m_pointLights.push_back(light);
  }

  void Renderer::AddDirectionalLight(const DirectionalLight& light)
  {
    m_directionalLights.push_back(light);
  }

  void Renderer::AddSpotLight(const SpotLight& light)
  {
    m_spotLights.push_back(light);
  }

  void Renderer::AddDebugCube(glm::mat4 position, glm::vec3 color)
  {
    m_debugCubes.push_back(DebugInstance(position, color));
  }

  void Renderer::AddDebugSphere(glm::mat4 position, glm::vec3 color)
  {
    m_debugSpheres.push_back(DebugInstance(position, color));
  }

  void Renderer::AddTime(double dt)
  {
    m_curTime += dt;
  }

  GLuint Renderer::LoadShader(const std::string &vsPath, const std::string &fsPath, const std::string &gsPath)
  {
    //Read the sources
    std::vector<char> vSrc(4096);
    {
      std::ifstream vsIs(vsPath, std::ios::in);
      if(!vsIs.is_open())
      {
        std::cerr << "Could not open vertex shader: " << vsPath << std::endl;
        return 0;
      }
      vsIs.read(&vSrc[0], vSrc.size());
      vsIs.close();
    }

    std::vector<char> fSrc(4096);
    {
      std::ifstream fsIs(fsPath, std::ios::in);
      if(!fsIs.is_open())
      {
        std::cerr << "Could not open fragment shader: " << fsPath << std::endl;
        return 0;
      }
      fsIs.read(&fSrc[0], fSrc.size());
      fsIs.close();
    }

    std::vector<char> gSrc(4096);
    if(!gsPath.empty())
    {
      std::ifstream gsIs(gsPath, std::ios::in);
      if(!gsIs.is_open())
      {
        std::cerr << "Could not open geometry shader: " << gsPath << std::endl;
        return 0;
      }
      gsIs.read(&gSrc[0], gSrc.size());
      gsIs.close();
    }

    //Build the shaders
    GLint status;

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    const char *vSrcPtr = &vSrc[0];
    glShaderSource(vs, 1, &vSrcPtr, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if(status != GL_TRUE)
    {
      GLint logLen;
      glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &logLen);
      std::vector<char> vLog(logLen);
      glGetShaderInfoLog(vs, logLen, NULL, &vLog[0]);
      std::cerr << "Vertex shader failed to compile: " << &vLog[0] << std::endl;
      glDeleteShader(vs);
      return 0;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fSrcPtr = &fSrc[0];
    glShaderSource(fs, 1, &fSrcPtr, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if(status != GL_TRUE)
    {
      GLint logLen;
      glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &logLen);
      std::vector<char> fLog(logLen);
      glGetShaderInfoLog(fs, logLen, NULL, &fLog[0]);
      std::cerr << "Fragment shader failed to compile: " << &fLog[0] << std::endl;
      glDeleteShader(vs);
      glDeleteShader(fs);
      return 0;
    }

    GLuint gs = 0;
    if(gSrc[0])
    {
      gs = glCreateShader(GL_GEOMETRY_SHADER);
      const char *gSrcPtr = &gSrc[0];
      glShaderSource(gs, 1, &gSrcPtr, NULL);
      glCompileShader(gs);
      glGetShaderiv(gs, GL_COMPILE_STATUS, &status);
      if(status != GL_TRUE)
      {
        GLint logLen;
        glGetShaderiv(gs, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> gLog(logLen);
        glGetShaderInfoLog(gs, logLen, NULL, &gLog[0]);
        std::cerr << "Geometry shader failed to compile: " << &gLog[0] << std::endl;
        glDeleteShader(vs);
        glDeleteShader(fs);
        glDeleteShader(gs);
        return 0;
      }
    }

    // Link the program
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    if(gs)
      glAttachShader(prog, gs);
    glLinkProgram(prog);

    // Check the program
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if(status != GL_TRUE)
    {
      GLint logLen;
      glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLen);
      std::vector<char> pLog(logLen > 0 ? logLen : 1);
      glGetProgramInfoLog(prog, logLen, NULL, &pLog[0]);
      std::cerr << "Shader failed to link: " << &pLog[0] << std::endl;
      glDeleteProgram(prog);
      prog = 0;
      //carry on
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    glDeleteShader(gs);
    return prog;
  }

  void Renderer::SetupGeometryPass()
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
    glDisable(GL_BLEND);
    glDepthFunc(GL_LESS);
    glClearColor(0.0,0.0,0.0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void Renderer::SetupLightPass()
  {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_compositeFBO);
    glClearColor(0.0,0.0,0.0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glDepthFunc(GL_ALWAYS);
    glBlendFunc(GL_ONE, GL_ONE);
  }

  void Renderer::SetupDebugPass()
  {
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
  }

  void Renderer::CompositeFrame()
  {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glClearColor(0.0,0.0,0.0,1);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_shdCompositor);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texComposite);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_texDepth);

    glUniform1i(glGetUniformLocation(m_shdCompositor, "sampBuffer"), 0);
    glUniform1i(glGetUniformLocation(m_shdCompositor, "sampDepth"), 1);

    glUniform1f(glGetUniformLocation(m_shdCompositor, "gamma"), m_gamma);
    glUniform2f(glGetUniformLocation(m_shdCompositor, "screenSize"), (float)m_width, (float)m_height);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_pPlane->m_vboVertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, m_pPlane->m_iStride, (void*)m_pPlane->m_iOffPos);
    glDrawArrays(GL_TRIANGLES, 0, m_pPlane->m_iNumTris*3);
    glDisableVertexAttribArray(0);
  }

  void Renderer::SetGlobalIllumination(glm::vec3 color)
  {
    m_globalIllumColor = color;
  }

  void Renderer::SetGamma(float gamma)
  {
    m_gamma = gamma;
  }

  void Renderer::ApplyGlobalIllumination()
  {

    glUseProgram(m_shdGlobalIllum);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texLambert);

    glUniform2f(glGetUniformLocation(m_shdGlobalIllum, "screenSize"), (float)m_width, (float)m_height);
    glUniform1i(glGetUniformLocation(m_shdGlobalIllum, "sampColor"), 0);
    glUniform3f(glGetUniformLocation(m_shdGlobalIllum, "lightColor"), (float)m_globalIllumColor.x, (float)m_globalIllumColor.y, (float)m_globalIllumColor.z);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_pPlane->m_vboVertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, m_pPlane->m_iStride, (void*)m_pPlane->m_iOffPos);

    glDrawArrays(GL_TRIANGLES, 0, m_pPlane->m_iNumTris*3);

    glDisableVertexAttribArray(0);
    glClear(GL_DEPTH_BUFFER_BIT);
  }
}
