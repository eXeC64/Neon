#include "Loader.hpp"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <glm/vec2.hpp>

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
