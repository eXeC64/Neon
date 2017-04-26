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
    m_curTime(0),
    m_viewYaw(0),
    m_viewTilt(0),
    m_shdMesh(0),
    m_shdLight(0),
    m_shdGlobalIllum(0),
    m_shdDebug(0),
    m_texLambert(0),
    m_texNormal(0),
    m_texPBRMaps(0),
    m_texDepth(0),
    m_FBO(0),
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
    if(m_shdLight)
      glDeleteProgram(m_shdLight);
    if(m_shdGlobalIllum)
      glDeleteProgram(m_shdGlobalIllum);
    if(m_shdDebug)
      glDeleteProgram(m_shdDebug);
    if(m_texLambert)
      glDeleteTextures(1, &m_texLambert);
    if(m_texNormal)
      glDeleteTextures(1, &m_texNormal);
    if(m_texPBRMaps)
      glDeleteTextures(1, &m_texPBRMaps);
    if(m_texDepth)
      glDeleteTextures(1, &m_texDepth);
    if(m_FBO)
      glDeleteFramebuffers(1, &m_FBO);
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
    {
      glDeleteTextures(1, &m_texLambert);
      glDeleteTextures(1, &m_texNormal);
      glDeleteTextures(1, &m_texPBRMaps);
      glDeleteTextures(1, &m_texDepth);
      glDeleteFramebuffers(1, &m_FBO);
      m_texLambert = 0;
      m_texNormal = 0;
      m_texPBRMaps = 0;
      m_texDepth = 0;
      m_FBO = 0;
      return false;
    }

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

    m_shdLight = LoadShader("shaders/light_vert.glsl", "shaders/light_frag.glsl");
    if(!m_shdLight)
      return false;

    m_shdGlobalIllum = LoadShader("shaders/globalillum_vert.glsl", "shaders/globalillum_frag.glsl");
    if(!m_shdGlobalIllum)
      return false;

    m_shdDebug = LoadShader("shaders/debug_vert.glsl", "shaders/debug_frag.glsl");
    if(!m_shdDebug)
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
    m_lights.clear();
    m_debugCubes.clear();
    m_debugSpheres.clear();
  }

  void Renderer::EndFrame()
  {
    //Prepare for geometry pass
    SetupGeometryPass();

    //Draw the geometry into the g buffers
    for (auto& model : m_models)
    {
      DrawMeshInstance(model);
    }

    //Prepare for lighting pass
    SetupLightPass();

    //Perform global illumination
    ApplyGlobalIllumination();

    //Apply all our lights
    for (auto& light : m_lights)
      DrawLightInstance(light);

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
    glm::mat4 proj = glm::perspective(20.0, 16.0/9.0, 0.1, 100.0);
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

  void Renderer::DrawMeshInstance(const MeshInstance &model)
  {
    glUseProgram(m_shdMesh);
    glUniformMatrix4fv(glGetUniformLocation(m_shdMesh, "matPos"), 1, GL_FALSE, &model.pos[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_shdMesh, "matView"), 1, GL_FALSE, &m_matProjection[0][0]);


    {
      Texture *pLambert = model.mat->m_pLambert;
      if(!pLambert)
        pLambert = m_pDefaultLambert;
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, pLambert->m_glTexture);
      glUniform1i(glGetUniformLocation(m_shdMesh, "sampLambert"), 0);
    }

    {
      Texture *pNormal = model.mat->m_pNormal;
      if(!pNormal)
        pNormal = m_pDefaultNormal;
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, pNormal->m_glTexture);
      glUniform1i(glGetUniformLocation(m_shdMesh, "sampNormal"), 1);
    }

    {
      Texture *pMetallic = model.mat->m_pMetallic;
      if(!pMetallic)
        pMetallic = m_pDefaultMetallic;
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, pMetallic->m_glTexture);
      glUniform1i(glGetUniformLocation(m_shdMesh, "sampMetallic"), 2);
    }

    {
      Texture *pRoughness = model.mat->m_pRoughness;
      if(!pRoughness)
        pRoughness = m_pDefaultRoughness;
      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, pRoughness->m_glTexture);
      glUniform1i(glGetUniformLocation(m_shdMesh, "sampRoughness"), 3);
    }

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

    glBindTexture(GL_TEXTURE_2D, 0);
  }

  void Renderer::DrawLightInstance(const LightInstance &light)
  {
    glUseProgram(m_shdLight);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texLambert);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_texNormal);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_texPBRMaps);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_texDepth);

    glUniformMatrix4fv(glGetUniformLocation(m_shdLight, "matView"), 1, GL_FALSE, &m_matProjection[0][0]);
    glUniform1f(glGetUniformLocation(m_shdLight, "time"), (float)m_curTime);
    glUniform3f(glGetUniformLocation(m_shdLight, "viewPos"), m_viewPos.x, m_viewPos.y, m_viewPos.z);
    glUniform2f(glGetUniformLocation(m_shdLight, "screenSize"), (float)m_width, (float)m_height);
    glUniform1i(glGetUniformLocation(m_shdLight, "sampLambert"), 0);
    glUniform1i(glGetUniformLocation(m_shdLight, "sampNormal"), 1);
    glUniform1i(glGetUniformLocation(m_shdLight, "sampPBRMaps"), 2);
    glUniform1i(glGetUniformLocation(m_shdLight, "sampDepth"), 3);
    glUniform3f(glGetUniformLocation(m_shdLight, "lightPos"), light.pos.x, light.pos.y, light.pos.z);
    glUniform3f(glGetUniformLocation(m_shdLight, "lightColor"), light.color.x, light.color.y, light.color.z);

    glBindVertexArray(m_pPlane->m_vaoConfig);
    glDrawArrays(GL_TRIANGLES, 0, m_pPlane->m_iNumTris*3);
    glBindVertexArray(0);
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

  void Renderer::AddLight(glm::vec3 pos, glm::vec3 color)
  {
    m_lights.push_back(LightInstance(pos, color));
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

  GLuint Renderer::LoadShader(const std::string &vsPath, const std::string &fsPath)
  {
    //Read the sources
    std::vector<char> vSrc(2048);
    std::ifstream vsIs(vsPath, std::ios::in);
    if(!vsIs.is_open())
    {
      std::cerr << "Could not open vertex shader: " << vsPath << std::endl;
      return 0;
    }
    vsIs.read(&vSrc[0], vSrc.size());
    vsIs.close();

    std::vector<char> fSrc(2048);
    std::ifstream fsIs(fsPath, std::ios::in);
    if(!fsIs.is_open())
    {
      std::cerr << "Could not open fragment shader: " << fsPath << std::endl;
      return 0;
    }
    fsIs.read(&fSrc[0], fSrc.size());
    fsIs.close();

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

    // Link the program
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
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
    return prog;
  }

  void Renderer::SetupGeometryPass()
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0,0.0,0.0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void Renderer::SetupLightPass()
  {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glClearColor(0.0,0.0,0.0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
  }

  void Renderer::SetupDebugPass()
  {
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDisable(GL_BLEND);
  }

  void Renderer::SetGlobalIllumination(glm::vec3 color)
  {
    m_globalIllumColor = color;
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
