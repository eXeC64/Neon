#pragma once

namespace ne
{
  class Texture;
  class Material
  {
    friend class Renderer;
  public:
    Material(
        Texture *pLambert = nullptr,
        Texture *pNormal = nullptr,
        Texture *pMetallic = nullptr,
        Texture *pRoughness = nullptr,
        Texture *pAO = nullptr
    )
      : m_pLambert(pLambert)
      , m_pNormal(pNormal)
      , m_pMetallic(pMetallic)
      , m_pRoughness(pRoughness)
      , m_pAO(pAO)
    {
    }
  private:
    Texture *m_pLambert;
    Texture *m_pNormal;
    Texture *m_pMetallic;
    Texture *m_pRoughness;
    Texture *m_pAO;
  };
}
