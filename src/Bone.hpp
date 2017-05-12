#pragma once

#include <glm/ext.hpp>
#include <string>
#include <vector>

namespace ne
{
  class Bone
  {
  public:
    size_t id;
    std::string name;
    glm::vec3 localPos;
    glm::quat localRot;
    glm::mat4 invTransform;
    std::vector<size_t> childIds;
  };
}
