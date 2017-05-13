#version 300 es

const int MAX_BONES = 32;

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec3 vertexNorm;
layout (location = 2) in vec2 vertexUV;
layout (location = 3) in vec4 boneWeights;
layout (location = 4) in vec4 boneIds;

uniform mat4 matPos;
uniform mat4 matLightProj;
uniform mat4 boneTransforms[MAX_BONES];

void main()
{
  vec4 localPos = vec4(0.0);

  for(int i = 0; i < 4; ++i)
  {
    vec4 pos = boneTransforms[int(boneIds[i])] * vec4(vertexPos, 1);
    localPos += pos * boneWeights[i];
  }

  gl_Position = matLightProj * matPos * localPos;
}
