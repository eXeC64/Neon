#include "Skeleton.hpp"

namespace ne
{

  void Skeleton::calculateTransforms(std::vector<glm::mat4>& outTrans) const
  {
    //Calculate local transformation matrix for each bone first
    for(size_t i = 0; i < bones.size(); ++i)
      outTrans[i] = glm::translate(bones[i].localPos) * glm::mat4_cast(bones[i].localRot);

    //Chain the transforms down the skeleton
    chainTransforms(glm::mat4(1.0), 0, outTrans);
  }

  void Skeleton::calculateInvTransforms(std::vector<glm::mat4>& outTrans) const
  {
    calculateTransforms(outTrans);

    //Now multiply by inverse transform
    for(size_t i = 0; i < bones.size(); ++i)
      outTrans[i] = outTrans[i] * bones[i].invTransform;
  }

  void Skeleton::chainTransforms(const glm::mat4& parentMat, size_t bone, std::vector<glm::mat4>& outTrans) const
  {
    outTrans[bone] = parentMat * outTrans[bone];
    for(auto& child : bones[bone].childIds)
      chainTransforms(outTrans[bone], child, outTrans);
  }

}
