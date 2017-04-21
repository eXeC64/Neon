#include "Loader.hpp"

#include "Material.hpp"
#include "Mesh.hpp"
#include "Model.hpp"
#include "Texture.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/vec2.hpp>
#include <png.h>
#include <vector>

namespace he
{

  Loader::Loader()
  {
  }

  Loader::~Loader()
  {
    for(auto it : m_textures)
      delete it.second;
    for(auto it : m_models)
      delete it.second;
  }

  Mesh* Loader::LoadMesh(const aiMesh* mesh)
  {
    std::vector<GLfloat> data;
    for(GLuint i = 0; i < mesh->mNumVertices; ++i)
    {
      data.push_back(mesh->mVertices[i].x);
      data.push_back(mesh->mVertices[i].y);
      data.push_back(mesh->mVertices[i].z);
      if(mesh->mTextureCoords[0])
      {
        data.push_back(mesh->mTextureCoords[0][i].x);
        data.push_back(mesh->mTextureCoords[0][i].y);
      }
      else
      {
        data.push_back(0.0f);
        data.push_back(0.0f);
      }
      data.push_back(mesh->mNormals[i].x);
      data.push_back(mesh->mNormals[i].y);
      data.push_back(mesh->mNormals[i].z);
    }

    std::vector<GLuint> indices;
    for(GLuint i = 0; i < mesh->mNumFaces; ++i)
    {
      for(GLuint j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
      {
        indices.push_back(mesh->mFaces[i].mIndices[j]);
      }
    }

    Mesh* pMesh = new Mesh();
    pMesh->m_iNumTris = data.size() / 24; //(8 floats per vertex, 3 vertices per tri)
    pMesh->m_iNumIndices = indices.size();
    pMesh->m_iStride = 8 * sizeof(GLfloat);
    pMesh->m_iOffPos = 0 * sizeof(GLfloat);
    pMesh->m_iOffUV = 3 * sizeof(GLfloat);
    pMesh->m_iOffNormal = 5 * sizeof(GLfloat);

    glGenVertexArrays(1, &pMesh->m_vaoConfig);
    glGenBuffers(1, &pMesh->m_vboVertices);
    glGenBuffers(1, &pMesh->m_vboIndices);

    glBindVertexArray(pMesh->m_vaoConfig);

    glBindBuffer(GL_ARRAY_BUFFER, pMesh->m_vboVertices);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), &data[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pMesh->m_vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, pMesh->m_iStride, (void*)pMesh->m_iOffPos);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, pMesh->m_iStride, (void*)pMesh->m_iOffUV);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, pMesh->m_iStride, (void*)pMesh->m_iOffNormal);

    glBindVertexArray(0);

    return pMesh;
  }

  void Loader::ProcessModelNode(Model* model, const aiScene* scene, const aiNode* node)
  {
    for(GLuint i = 0; i < node->mNumMeshes; ++i)
    {
      const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
      Mesh* heMesh = LoadMesh(mesh);
      model->m_meshes.push_back(heMesh);

      const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
      Texture *diffuse = nullptr;
      Texture *normal = nullptr;
      if(material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
      {
        aiString str;
        material->GetTexture(aiTextureType_DIFFUSE, 0, &str);
        std::string path(str.C_Str());
        diffuse = LoadPNG(path);
      }
      if(material->GetTextureCount(aiTextureType_NORMALS) > 0)
      {
        aiString str;
        material->GetTexture(aiTextureType_NORMALS, 0, &str);
        std::string path(str.C_Str());
        normal = LoadPNG(path);
      }
      model->m_materials.push_back(new Material(diffuse, normal));
    }
    for(GLuint i = 0; i < node->mNumChildren; ++i)
    {
      ProcessModelNode(model, scene, node->mChildren[i]);
    }
  }

  Model* Loader::LoadModel(const std::string &path)
  {
    //Check cache
    {
      auto it = m_models.find(path);
      if(it != m_models.end()) {
        return it->second;
      }
    }

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals |
        aiProcess_PreTransformVertices | aiProcess_SplitLargeMeshes |
        aiProcess_RemoveRedundantMaterials | aiProcess_GenUVCoords);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
      return nullptr;

    Model* model = new Model();
    ProcessModelNode(model, scene, scene->mRootNode);

    m_models[path] = model;
    return model;
  }

  Texture* Loader::LoadPNG(const std::string &path)
  {
    {
      auto it = m_textures.find(path);
      if(it != m_textures.end()) {
        return it->second;
      }
    }

    FILE *fp = fopen(path.c_str(), "rb");

    if(!fp)
      return nullptr;

    png_structp pPNG = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!pPNG)
    {
      fclose(fp);
      return nullptr;
    }

    png_infop pInfo = png_create_info_struct(pPNG);
    if(!pInfo)
    {
      fclose(fp);
      png_destroy_read_struct(&pPNG, NULL, NULL);
      return nullptr;
    }

    png_init_io(pPNG, fp);
    png_read_png(pPNG, pInfo, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL);
    png_uint_32 width, height;
    png_get_IHDR(pPNG, pInfo, &width, &height, NULL, NULL, NULL, NULL, NULL);
    unsigned int bytesPerRow = png_get_rowbytes(pPNG, pInfo);
    std::vector<char> data(bytesPerRow*height);
    png_bytepp rowPointers = png_get_rows(pPNG, pInfo);

    for (unsigned int i = 0; i < height; i++)
      memcpy(&data[0]+(bytesPerRow * (height-1-i)), rowPointers[i], bytesPerRow);

    png_destroy_read_struct(&pPNG, &pInfo, NULL);

    fclose(fp);

    Texture *pTex = new Texture();
    pTex->m_width = width;
    pTex->m_height = width;
    glGenTextures(1, &pTex->m_glTexture);
    glBindTexture(GL_TEXTURE_2D, pTex->m_glTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, &data[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_textures[path] = pTex;
    return pTex;
  }

  Mesh* Loader::GeneratePlane()
  {
    Mesh *pMesh = new Mesh();
    pMesh->m_iNumTris = 2;
    pMesh->m_iStride = 8 * sizeof(GLfloat);
    pMesh->m_iOffPos = 0 * sizeof(GLfloat);
    pMesh->m_iOffUV = 3 * sizeof(GLfloat);
    pMesh->m_iOffNormal = 5 * sizeof(GLfloat);

    std::vector<GLfloat> data = {
      -1,-1,0, 0,0, 0,0,1,
       1, 1,0, 1,1, 0,0,1,
      -1, 1,0, 0,1, 0,0,1,
      -1,-1,0, 0,0, 0,0,1,
       1,-1,0, 1,0, 0,0,1,
       1, 1,0, 1,1, 0,0,1
    };

    glGenBuffers(1, &pMesh->m_vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, pMesh->m_vboVertices);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), &data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return pMesh;
  }

}
