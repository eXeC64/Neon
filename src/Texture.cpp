#include "Texture.hpp"

namespace he
{
  Texture::Texture() : m_glTexture(0), m_width(0), m_height(0) {};

  Texture::~Texture()
  {
    if(m_glTexture)
      glDeleteTextures(1, &m_glTexture);
  }

  int Texture::Width()
  {
    return m_width;
  }

  int Texture::Height()
  {
    return m_height;
  }
}
