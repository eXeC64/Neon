#version 300 es

precision highp float;

uniform sampler2D sampDiffuse;
uniform sampler2D sampNormal;

in vec2 inUV;
in mat3 inNormalMat;

layout (location = 0) out vec3 outDiffuse;
layout (location = 1) out vec3 outNormal;

void main()
{
  outDiffuse = texture(sampDiffuse, inUV).rgb;
  outNormal = texture(sampNormal, inUV).xyz * inNormalMat;
}
