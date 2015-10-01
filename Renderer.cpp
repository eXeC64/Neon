#include "Renderer.hpp"

#include "Mesh.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

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
    m_shader = LoadShader("simple.vs", "simple.fs");

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

    glUseProgram(m_shader);
    GLuint uPos = glGetUniformLocation(m_shader, "pos");
    GLuint uView = glGetUniformLocation(m_shader, "view");
    glUniformMatrix4fv(uPos, 1, GL_FALSE, &matPosition[0][0]);
    glUniformMatrix4fv(uView, 1, GL_FALSE, &m_matProjection[0][0]);

    glDrawArrays(GL_TRIANGLES, 0, pMesh->m_iNumTris*3);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
  }

  void Renderer::AddLight(glm::vec3 pos, glm::vec3 rgb, double radius)
  {
    //add this light to the list to be rendered this frame
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
