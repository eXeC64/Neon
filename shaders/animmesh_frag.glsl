#version 300 es

precision highp float;

uniform sampler2D sampLambert;
uniform sampler2D sampNormal;
uniform sampler2D sampMetallic;
uniform sampler2D sampRoughness;

in vec2 inUV;
in mat3 inNormalMat;

layout (location = 0) out vec3 outLambert;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec2 outPBRMaps;

void main()
{
  outLambert = texture(sampLambert, inUV).rgb;

  //Fix the range of the normal from [0,1] to [-1,-1] for calculations
  vec3 rangeCorrectedNormal = texture(sampNormal, inUV).xyz * 2.0 - 1.0;
  // Textures are flipped, so normals need to be too
  vec3 flippedNormal = vec3(-1, -1, 1) * rangeCorrectedNormal;
  //Transform the normal by the normal of the polygon its attached to
  outNormal = flippedNormal * inNormalMat;
  outPBRMaps.r = texture(sampMetallic, inUV).r;
  outPBRMaps.g = texture(sampRoughness, inUV).r;
}
