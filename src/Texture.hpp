#pragma once

#include "OpenGL.hpp"

namespace he
{
  class Texture
  {
    friend class Renderer;
    friend class Loader;
  public:
    ~Texture();
    int Width();
    int Height();
  private:
    Texture();
    GLuint m_glTexture;
    int m_width;
    int m_height;
  };
}
