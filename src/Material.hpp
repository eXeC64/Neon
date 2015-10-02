#pragma once

namespace he
{
  class Texture;
  class Material
  {
    friend class Renderer;
  public:
    Material(Texture *pDiffuse = nullptr, Texture *pNormal = nullptr)
    {
      m_pDiffuse = pDiffuse;
      m_pNormal = pNormal;
    }
  private:
    Texture *m_pDiffuse;
    Texture *m_pNormal;
  };
}
