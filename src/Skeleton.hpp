#pragma once

#include "Bone.hpp"
#include <vector>
#include <unordered_map>

namespace ne
{
  class Skeleton
  {
  public:
    std::vector<Bone> bones;
    void calculateTransforms(std::vector<glm::mat4>& outTrans) const;
    void calculateInvTransforms(std::vector<glm::mat4>& outTrans) const;
  private:
    void chainTransforms(const glm::mat4& parentMat, size_t bone, std::vector<glm::mat4>& outTrans) const;
  };
}
