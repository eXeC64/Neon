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
  outNormal = (texture(sampNormal, inUV).xyz * 2.0 - 1.0) * inNormalMat;
  outPBRMaps.r = texture(sampMetallic, inUV).r;
  outPBRMaps.g = texture(sampRoughness, inUV).r;
}
