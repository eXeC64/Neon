#pragma once

#include "OpenGL.hpp"

#include <imgui.h>
#include <SDL2/SDL.h>

namespace ne
{

  class ImguiWrapper
  {
  public:
    ImguiWrapper();
    ~ImguiWrapper();

    void NewFrame(SDL_Window *window);
    void Render();
    void HandleEvent(const SDL_Event *e);
    bool UsingMouse();
    bool UsingKeyboard();

  private:
    double m_time;
    bool m_mousePressed[3];
    float m_mouseWheel;
    
    GLuint m_texFont;
    GLuint m_shader;
    GLuint m_vboVertices;
    GLuint m_vboIndices;
  };
}
