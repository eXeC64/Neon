#pragma once

namespace he
{
  class Texture;
  class Material
  {
    friend class Renderer;
  public:
    Material(
        Texture *pDiffuse = nullptr,
        Texture *pNormal = nullptr,
        Texture *pLambert = nullptr,
        Texture *pMetallic = nullptr,
        Texture *pRoughness = nullptr,
        Texture *pAO = nullptr
    )
      : m_pDiffuse(pDiffuse)
      , m_pNormal(pNormal)
      , m_pLambert(pLambert)
      , m_pMetallic(pMetallic)
      , m_pRoughness(pRoughness)
      , m_pAO(pAO)
    {
    }
  private:
    Texture *m_pDiffuse;
    Texture *m_pNormal;
    Texture *m_pLambert;
    Texture *m_pMetallic;
    Texture *m_pRoughness;
    Texture *m_pAO;
  };
}
