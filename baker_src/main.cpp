#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/ext.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <queue>

/*
  A file format for a skeleton:

  u32 boneCount
  {
    u8  id
    f32 pos[3] //position vector (relative to parent)
    f32 rot[4] //rotation quaternion (relative to parent)
    f32 invMat[16] //row-major
    u8  nameLen
    u8  name[nameLen] //no null terminator
    u8  numChildren
    u8  childIds[numChildren]
  } [boneCount]

  A file format for a mesh:
  
  u8  flags (1: hasSkeleton)
  u32 numVerts
  u32 numIndices
  {
    f32 pos[3]
    f32 normal[3]
    f32 uv[2]
    f32 boneWeights[4] //if hasSkeleton
    f32 boneIds[4]     //if hasSkeleton
  } [numVerts]
  u32 index[numIndices]


  A file format for an animation clip

  u32 channelCount
  {
    u8  boneId
    u32 numKeys
    {
      f64 time   //keys sorted chronologically
      f32 pos[3] //xyz
      f32 rot[4] //quat
    } [numKeys]
  } [channelCount]

  TODO - write a program to bake collada files to animations

*/
struct Bone
{
  size_t id;
  glm::quat localRot;
  glm::vec4 localPos;
  glm::mat4 invMat;
  std::string name;
  std::vector<size_t> childIds;
};

void copyMatrix(const aiMatrix4x4t<float>& from, glm::mat4& to)
{
  to[0][0] = from.a1; to[1][0] = from.a2;
  to[2][0] = from.a3; to[3][0] = from.a4;
  to[0][1] = from.b1; to[1][1] = from.b2;
  to[2][1] = from.b3; to[3][1] = from.b4;
  to[0][2] = from.c1; to[1][2] = from.c2;
  to[2][2] = from.c3; to[3][2] = from.c4;
  to[0][3] = from.d1; to[1][3] = from.d2;
  to[2][3] = from.d3; to[3][3] = from.d4;

  /* to[0][0] = from.a1; to[0][1] = from.a2; */
  /* to[0][2] = from.a3; to[0][3] = from.a4; */
  /* to[1][0] = from.b1; to[1][1] = from.b2; */
  /* to[1][2] = from.b3; to[1][3] = from.b4; */
  /* to[2][0] = from.c1; to[2][1] = from.c2; */
  /* to[2][2] = from.c3; to[2][3] = from.c4; */
  /* to[3][0] = from.d1; to[3][1] = from.d2; */
  /* to[3][2] = from.d3; to[3][3] = from.d4; */
}

void writeU8(std::ostream &out, uint8_t value)
{
  out.put(value);
}

void writeU32(std::ostream &out, uint32_t value)
{
  out.write((char*)&value, 4);
}

void writeF32(std::ostream &out, float value)
{
  out.write((char*)&value, 4);
}

void writeBytes(std::ostream &out, const char* bytes, size_t num)
{
  out.write(bytes, num);
}

void writeM44(std::ostream &out, const glm::mat4& matrix)
{
  writeBytes(out, (char*)&matrix[0][0], 16 * 4);
}

uint8_t readU8(std::istream &in)
{
  return in.get();
}

uint32_t readU32(std::istream &in)
{
  uint32_t ret;
  in.read((char*)&ret, 4);
  return ret;
}

float readF32(std::istream &in)
{
  float ret;
  in.read((char*)&ret, 4);
  return ret;
}

void readBytes(std::istream &in, char* buf, size_t num)
{
  in.read(buf, num);
}

glm::mat4 readM44(std::istream &in)
{
  glm::mat4 ret;
  readBytes(in, (char*)&ret[0][0], 16 * 4);
  return ret;
}

bool bakeSkeleton(const std::string& outFile, const std::string& path, const std::string& nodeName)
{
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(path, 0);

  if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    return false;

  const aiNode* skelNode = scene->mRootNode->FindNode(nodeName.c_str());

  if(!skelNode)
  {
    std::cerr << "Could not find node: '" << nodeName << "'" << std::endl;
    return false;
  }

  if(skelNode->mNumChildren != 1)
  {
    std::cerr << "Node have root bone as only child" << std::endl;
    return false;
  }

  const aiNode* rootBone = skelNode->mChildren[0];

  std::unordered_map<std::string, glm::mat4> boneInvTransforms;
  for(size_t i = 0; i < scene->mNumMeshes; ++i)
  {
    const aiMesh* mesh = scene->mMeshes[i];
    for(size_t j = 0; j < mesh->mNumBones; ++j)
    {
      const aiBone* bone = mesh->mBones[j];
      const std::string boneName = bone->mName.C_Str();
      glm::mat4 bonePos;
      copyMatrix(bone->mOffsetMatrix, bonePos);
      boneInvTransforms[boneName] = bonePos;
    }
  }

  //Have parent of root bone & bone positions, build skeleton
  std::vector<Bone> bones;
  std::queue<const aiNode*> nodeQueue;
  nodeQueue.push(rootBone);

  while(!nodeQueue.empty())
  {
    const aiNode* curNode = nodeQueue.front();
    nodeQueue.pop();

    Bone bone;
    bone.id = bones.size();
    bone.name = curNode->mName.C_Str();
    glm::mat4 localMat;
    copyMatrix(curNode->mTransformation, localMat);
    bone.localRot = glm::quat_cast(localMat);
    bone.localPos = localMat * glm::vec4(0.0, 0.0, 0.0, 1.0);
    bone.invMat = boneInvTransforms[bone.name];
    bones.push_back(bone);

    //Add this bone to its parent's children list, if it's not the root node
    if(curNode != rootBone)
    {
      const aiNode* parent = curNode->mParent;
      size_t parentId = 0;
      while(bones[parentId].name != parent->mName.C_Str())
      {
        ++parentId;
        if(parentId >= bones.size())
        {
          std::cerr << "Couldn't find parent bone: " << parent->mName.C_Str() << std::endl;
          return false;
        }
      }
      bones[parentId].childIds.push_back(bone.id);
    }

    //Add children of node to queue
    for(size_t i = 0; i < curNode->mNumChildren; ++i)
      nodeQueue.push(curNode->mChildren[i]);
  }

  std::cout << "Extracted skeleton of " << bones.size() << " bones" << std::endl;

  std::ofstream outStream(outFile);

  writeU32(outStream, bones.size());
  for(auto& bone : bones)
  {
    std::cout << bone.id << " = " << bone.name
              << "   " << bone.localPos.x
              << "   " << bone.localPos.y
              << "   " << bone.localPos.z << std::endl;
    writeU8(outStream, bone.id);
    writeF32(outStream, bone.localPos.x);
    writeF32(outStream, bone.localPos.y);
    writeF32(outStream, bone.localPos.z);
    writeF32(outStream, bone.localRot[0]);
    writeF32(outStream, bone.localRot[1]);
    writeF32(outStream, bone.localRot[2]);
    writeF32(outStream, bone.localRot[3]);
    writeM44(outStream, bone.invMat);
    writeU8(outStream, bone.name.length());
    writeBytes(outStream, bone.name.c_str(), bone.name.length());
    writeU8(outStream, bone.childIds.size());
    for(auto& id : bone.childIds)
      writeU8(outStream, id);
  }
  return true;
}

bool bakeStaticMesh(const std::string& outFile, const std::string& path, const std::string& meshName)
{
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(path,
      aiProcess_Triangulate | aiProcess_FlipUVs |
      aiProcess_GenNormals | aiProcess_GenUVCoords);

  if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    return false;

  const aiNode* meshNode = scene->mRootNode->FindNode(meshName.c_str());

  if(!meshNode)
  {
    std::cerr << "Could not find node: '" << meshName << "'" << std::endl;
    return false;
  }

  if(meshNode->mNumMeshes != 1)
  {
    std::cerr << "Given node must have exactly one mesh" << std::endl;
    return false;
  }

  const aiMesh* mesh = scene->mMeshes[meshNode->mMeshes[0]];
  if(!mesh)
  {
    std::cerr << "Could not find mesh: '" << meshName << "'" << std::endl;
    return false;
  }

  std::vector<size_t> indices;
  for(size_t i = 0; i < mesh->mNumFaces; ++i)
  {
    for(size_t j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
    {
      indices.push_back(mesh->mFaces[i].mIndices[j]);
    }
  }

  std::cout << "Extracted static mesh" << std::endl;

  std::ofstream outStream(outFile);

  writeU8(outStream, 0); //No flags set
  writeU32(outStream, mesh->mNumVertices);
  writeU32(outStream, indices.size());
  for(size_t i = 0; i < mesh->mNumVertices; ++i)
  {
    writeF32(outStream, mesh->mVertices[i].x);
    writeF32(outStream, mesh->mVertices[i].y);
    writeF32(outStream, mesh->mVertices[i].z);
    writeF32(outStream, mesh->mNormals[i].x);
    writeF32(outStream, mesh->mNormals[i].y);
    writeF32(outStream, mesh->mNormals[i].z);
    writeF32(outStream, mesh->mTextureCoords[0][i].x);
    writeF32(outStream, mesh->mTextureCoords[0][i].y);
  }
  for(auto& index : indices)
    writeU32(outStream, index);
  std::cout << "Written static mesh" << std::endl;

  return true;
}

bool loadSkeletonBoneIds(const std::string& path, std::unordered_map<std::string,size_t>& outIds)
{
  std::ifstream in(path);

  if(!in.good())
    return false;

  const size_t boneCount = readU32(in);

  for(size_t i = 0; i < boneCount; ++i)
  {
    const size_t id = readU8(in);
    for (size_t j = 0; j < 7; ++j)
      readF32(in);
    readM44(in);
    std::string name(readU8(in), ' ');
    readBytes(in, &name[0], name.length());
    const uint8_t numChildren = readU8(in);

    for(size_t j = 0; j < numChildren; ++j)
      readU8(in);

    outIds[name] = id;
  }
  return true;
}

bool bakeSkeletelMesh(const std::string& outFile, const std::string& path, const std::string& meshName, const std::string& skeletonPath)
{
  std::unordered_map<std::string,size_t> boneIds;

  if(!loadSkeletonBoneIds(skeletonPath, boneIds))
  {
    std::cerr << "Could not load skeleton from: " << skeletonPath << std::endl;
    return false;
  }

  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(path,
      aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_LimitBoneWeights |
      aiProcess_GenNormals | aiProcess_GenUVCoords);

  if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    return false;

  const aiNode* meshNode = scene->mRootNode->FindNode(meshName.c_str());

  if(!meshNode)
  {
    std::cerr << "Could not find node: '" << meshName << "'" << std::endl;
    return false;
  }

  if(meshNode->mNumMeshes != 1)
  {
    std::cerr << "Given node must have exactly one mesh" << std::endl;
    return false;
  }

  const aiMesh* mesh = scene->mMeshes[meshNode->mMeshes[0]];
  if(!mesh)
  {
    std::cerr << "Could not find mesh: '" << meshName << "'" << std::endl;
    return false;
  }


  //Prepare lookup table for bone weights
  //vertex id -> bone id or weight)
  std::unordered_map<size_t, std::vector<float> > boneWeights;
  std::unordered_map<size_t, std::vector<float> > boneWeightIds;
  for(size_t i = 0; i < mesh->mNumBones; ++i)
  {
    const aiBone *bone = mesh->mBones[i];

    const size_t boneId = boneIds[bone->mName.C_Str()];
    for(size_t w = 0; w < bone->mNumWeights; ++w)
    {
      const aiVertexWeight *weight = &bone->mWeights[w];
      boneWeights[weight->mVertexId].push_back(weight->mWeight);
      boneWeightIds[weight->mVertexId].push_back(boneId);
    }
  }

  std::vector<size_t> indices;
  for(size_t i = 0; i < mesh->mNumFaces; ++i)
  {
    for(size_t j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
    {
      indices.push_back(mesh->mFaces[i].mIndices[j]);
    }
  }

  std::cout << "Extracted skeletel mesh" << std::endl;

  std::ofstream outStream(outFile);

  writeU8(outStream, 1); //Is skeletel mesh
  writeU32(outStream, mesh->mNumVertices);
  writeU32(outStream, indices.size());
  for(size_t i = 0; i < mesh->mNumVertices; ++i)
  {
    writeF32(outStream, mesh->mVertices[i].x);
    writeF32(outStream, mesh->mVertices[i].y);
    writeF32(outStream, mesh->mVertices[i].z);
    writeF32(outStream, mesh->mNormals[i].x);
    writeF32(outStream, mesh->mNormals[i].y);
    writeF32(outStream, mesh->mNormals[i].z);
    writeF32(outStream, mesh->mTextureCoords[0][i].x);
    writeF32(outStream, mesh->mTextureCoords[0][i].y);

    const std::vector<float>& weights = boneWeights[i];
    const std::vector<float>& ids = boneWeightIds[i];

    for(auto weight : weights)
      writeF32(outStream, weight);
    for(size_t i = 0; i < 4 - weights.size(); ++i)
      writeF32(outStream, 0.0f);

    for(auto id : ids)
    {
      writeF32(outStream, (float)id);
    }
    for(size_t i = 0; i < 4 - ids.size(); ++i)
      writeF32(outStream, 0.0f);
  }

  for(auto& index : indices)
    writeU32(outStream, index);

  std::cout << "Written skeletel mesh" << std::endl;
  return true;
}

int main(int argc, char** argv)
{
  bakeSkeleton("cowboy.skel", "meshes/cowboy.dae", "Armature");
  /* bakeStaticMesh("cowboy.mesh", "meshes/cowboy.dae", "Cube"); */
  bakeSkeletelMesh("cowboy.mesh", "meshes/cowboy.dae", "Cube", "cowboy.skel");
  return 0;
}

