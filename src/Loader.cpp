#include "Loader.hpp"

#include "Mesh.hpp"
#include "Texture.hpp"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <glm/vec2.hpp>
#include <png.h>
#include <stdio.h>
#include <string.h>

namespace
{
  glm::ivec3 parseTrip(const std::string &str)
  {
    //These indices are 0 = invalid, 1 = first
    glm::ivec3 ret(0,0,0);

    size_t seps = std::count(str.begin(), str.end(), '/');
    std::istringstream ss(str);
    char junk;

    if(seps == 0)
    {
      ss >> ret[0];
    }
    else if(seps == 1)
    {
      ss >> ret[0] >> junk >> ret[1];
    }
    else if(seps == 2)
    {
      ss >> ret[0] >> junk;
      if(ss.peek() != '/')
        ss >> ret[1];
      ss >> junk >> ret[2];
    }
    return ret;
  }
}

namespace he
{
  Mesh* Loader::LoadOBJ(const std::string &path)
  {
    std::ifstream fin;
    fin.open(path);

    if(!fin.good())
      return nullptr;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<glm::ivec3> indices;

    while(!fin.eof())
    {
      std::string line;
      std::getline(fin, line, '\n');

      if(line.empty() || line[0] == '#')
        continue;

      std::string cmd;
      std::istringstream ls(line);
      std::getline(ls, cmd, ' ');

      if(cmd == "v")
      {
        double x,y,z;
        ls >> x >> y >> z;
        vertices.push_back(glm::vec3(x,y,z));
      }
      else if(cmd == "vt")
      {
        double u,v;
        ls >> u >> v;
        uvs.push_back(glm::vec2(u,v));
      }
      else if(cmd == "vn")
      {
        double x,y,z;
        ls >> x >> y >> z;
        normals.push_back(glm::vec3(x,y,z));
      }
      else if(cmd == "f")
      {
        std::string faceVertex[3];
        std::getline(ls, faceVertex[0], ' ');
        std::getline(ls, faceVertex[1], ' ');
        std::getline(ls, faceVertex[2], '\n');

        for(int i = 0; i < 3; ++i)
        {
          glm::ivec3 idx = parseTrip(faceVertex[i]);
          //Subtract 1 from each index so it's 0-based and -1 = invalid
          idx -= glm::ivec3(1,1,1);
          indices.push_back(idx);
        }
      }
    }

    //For now just construct an un-indexed mesh: x,y,z,u,v,nx,ny,nz
    std::vector<GLfloat> data;
    for(auto idx : indices)
    {
      //Face indices must always be valid
      for(int i = 0; i < 3; ++i)
        data.push_back(vertices[idx[0]][i]);

      //Do we have a valid uv index?
      if(idx[1] >= 0)
        for(int i = 0; i < 2; ++i)
          data.push_back(uvs[idx[1]][i]);
      else
        for(int i = 0; i < 2; ++i)
          data.push_back(0); //Just set tex coords to 0

      //Do we have valid normal index?
      if(idx[2] >= 0)
        for(int i = 0; i < 3; ++i)
          data.push_back(normals[idx[2]][i]);
      else
        for(int i = 0; i < 3; ++i)
          data.push_back(i == 0 ? 1 : 0); //Just point towards +x instead
    }

    Mesh *pMesh = new Mesh();
    pMesh->m_iNumTris = data.size() / 24; //(8 floats per vertex, 3 vertices per tri)
    pMesh->m_iStride = 8 * sizeof(GLfloat);
    pMesh->m_iOffPos = 0 * sizeof(GLfloat);
    pMesh->m_iOffUV = 3 * sizeof(GLfloat);
    pMesh->m_iOffNormal = 5 * sizeof(GLfloat);

    glGenBuffers(1, &pMesh->m_vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, pMesh->m_vboVertices);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), &data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return pMesh;
  }

  Texture* Loader::LoadPNG(const std::string &path)
  {
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

    return pTex;
  }

  Mesh* Loader::Plane()
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
