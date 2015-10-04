#include "Renderer.hpp"

#include "Mesh.hpp"
#include "Material.hpp"
#include "Texture.hpp"
#include "Loader.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

namespace he
{

  Renderer::Renderer() :
    m_bIsInit(false),
    m_width(0), m_height(0),
    m_shdMesh(0),
    m_shdLight(0),
    m_texDiffuse(0),
    m_texNormal(0),
    m_texWorldPos(0),
    m_texDepth(0),
    m_FBO(0),
    m_pPlane(nullptr)
  {};

  Renderer::~Renderer()
  {
    if(m_shdMesh)
      glDeleteProgram(m_shdMesh);
    if(m_shdLight)
      glDeleteProgram(m_shdLight);
    if(m_texDiffuse)
      glDeleteTextures(1, &m_texDiffuse);
    if(m_texNormal)
      glDeleteTextures(1, &m_texNormal);
    if(m_texWorldPos)
      glDeleteTextures(1, &m_texWorldPos);
    if(m_texDepth)
      glDeleteTextures(1, &m_texDepth);
    if(m_FBO)
      glDeleteFramebuffers(1, &m_FBO);
    if(m_pPlane)
      delete m_pPlane;
  }

  bool Renderer::Init(int width, int height)
  {
    m_width = width;
    m_height = height;

    //Construct a frame buffer
    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);

    glGenTextures(1, &m_texDiffuse);
    glGenTextures(1, &m_texNormal);
    glGenTextures(1, &m_texWorldPos);
    glGenTextures(1, &m_texDepth);

    glBindTexture(GL_TEXTURE_2D, m_texDiffuse);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texDiffuse, 0);

    glBindTexture(GL_TEXTURE_2D, m_texNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_texNormal, 0);

    glBindTexture(GL_TEXTURE_2D, m_texWorldPos);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_texWorldPos, 0);

    glBindTexture(GL_TEXTURE_2D, m_texDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_texDepth, 0);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, drawBuffers);

    glBindTexture(GL_TEXTURE_2D, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      glDeleteTextures(1, &m_texDiffuse);
      glDeleteTextures(1, &m_texNormal);
      glDeleteTextures(1, &m_texWorldPos);
      glDeleteTextures(1, &m_texDepth);
      glDeleteFramebuffers(1, &m_FBO);
      m_texDiffuse = 0;
      m_texNormal = 0;
      m_texWorldPos = 0;
      m_texDepth = 0;
      m_FBO = 0;
      return false;
    }

    //Return to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(0.5,0.5,0.5,1);
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

    m_pPlane = Loader::Plane();
    if(!m_pPlane)
      return false;

    m_bIsInit = true;
    return true;
  }

  void Renderer::BeginFrame()
  {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);

    glClearColor(0.0,0.0,0.0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void Renderer::EndFrame()
  {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    if(false /* debug mode 1 */)
    {
      glClearColor(0.5,0.5,0.5,1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glReadBuffer(GL_COLOR_ATTACHMENT0);
      glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width/2, m_height/2, GL_COLOR_BUFFER_BIT, GL_LINEAR);

      glReadBuffer(GL_COLOR_ATTACHMENT1);
      glBlitFramebuffer(0, 0, m_width, m_height, 640, 0, m_width, m_height/2, GL_COLOR_BUFFER_BIT, GL_LINEAR);

      glReadBuffer(GL_COLOR_ATTACHMENT2);
      glBlitFramebuffer(0, 0, m_width, m_height, 0, m_height/2, m_width/2, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    }
    else
    {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glUseProgram(m_shdLight);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, m_texDiffuse);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, m_texNormal);
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, m_texWorldPos);

      glUniform2f(glGetUniformLocation(m_shdLight, "screenSize"), (float)m_width, (float)m_height);
      glUniform1i(glGetUniformLocation(m_shdLight, "sampDiffuse"), 0);
      glUniform1i(glGetUniformLocation(m_shdLight, "sampNormal"), 1);
      glUniform1i(glGetUniformLocation(m_shdLight, "sampPosition"), 2);

      glEnableVertexAttribArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, m_pPlane->m_vboVertices);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, m_pPlane->m_iStride, (void*)m_pPlane->m_iOffPos);
      glDrawArrays(GL_TRIANGLES, 0, m_pPlane->m_iNumTris*3);
      glDisableVertexAttribArray(0);
    }
  }

  void Renderer::SetProjectionMatrix(glm::mat4 matProjection)
  {
    m_matProjection = matProjection;
  }

  void Renderer::AddMesh(Mesh *pMesh, Material *pMat, glm::mat4 matPosition)
  {
    if(!pMesh || !pMat)
      return;

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, pMesh->m_vboVertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, pMesh->m_iStride, (void*)pMesh->m_iOffPos);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, pMesh->m_iStride, (void*)pMesh->m_iOffUV);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, pMesh->m_iStride, (void*)pMesh->m_iOffNormal);

    glUseProgram(m_shdMesh);
    glUniformMatrix4fv(glGetUniformLocation(m_shdMesh, "matPos"), 1, GL_FALSE, &matPosition[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_shdMesh, "matView"), 1, GL_FALSE, &m_matProjection[0][0]);

    Texture *pDiffuse = pMat->m_pDiffuse;
    if(pDiffuse)
    {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, pMat->m_pDiffuse->m_glTexture);
      glUniform1i(glGetUniformLocation(m_shdMesh, "sampDiffuse"), 0);
    }

    Texture *pNormal = pMat->m_pNormal;
    if(pNormal)
    {
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, pMat->m_pNormal->m_glTexture);
      glUniform1i(glGetUniformLocation(m_shdMesh, "sampNormal"), 1);
    }

    glDrawArrays(GL_TRIANGLES, 0, pMesh->m_iNumTris*3);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
  }

  void Renderer::AddLight(glm::vec3 pos, glm::vec3 rgb, double radius)
  {
    //add this light to the list to be rendered this frame
    (void)pos;
    (void)rgb;
    (void)radius;
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
}
