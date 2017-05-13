#pragma once

#include <glm/ext.hpp>
#include <vector>

namespace ne
{
  class Skeleton;

  struct Keyframe
  {
    double time;
    glm::vec3 position;
    glm::quat rotation;
  };

  struct AnimationChannel
  {
    size_t boneId;
    std::vector<Keyframe> keyframes;
  };

  class Animation
  {
    friend class Loader;

  public:
    ~Animation();

    void apply(Skeleton* skeleton, double time);
    double duration() const { return m_duration; }

  private:
    Animation();
    std::vector<AnimationChannel> m_channels;
    double m_duration;
  };
}
