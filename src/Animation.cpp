#include "Animation.hpp"

#include "Skeleton.hpp"

#include <iostream>

namespace ne
{

  Animation::Animation()
    : m_duration(0.0)
  {}

  Animation::~Animation()
  {}

  void Animation::apply(Skeleton* skeleton, double time)
  {
    for(auto& channel : m_channels)
    {
      //Find the next frame
      size_t nextFrame = 0;
      while(nextFrame + 1 < channel.keyframes.size() && time > channel.keyframes[nextFrame].time)
        ++nextFrame;

      size_t prevFrame = nextFrame == 0 ? 0 : nextFrame - 1;

      const double nextFrameTime = channel.keyframes[nextFrame].time;
      const double prevFrameTime = channel.keyframes[prevFrame].time;

      const double relTime = (time - prevFrameTime) / (nextFrameTime - prevFrameTime);

      skeleton->bones[channel.boneId].localPos = glm::mix(
          channel.keyframes[prevFrame].position,
          channel.keyframes[nextFrame].position,
          (float)relTime);

      skeleton->bones[channel.boneId].localRot = glm::slerp(
          channel.keyframes[prevFrame].rotation,
          channel.keyframes[nextFrame].rotation,
          (float)relTime);
    }
  }

}
