#version 300 es

precision highp float;

uniform sampler2D sampLambert;
uniform sampler2D sampNormal;

in vec2 inUV;
in mat3 inNormalMat;

layout (location = 0) out vec3 outLambert;
layout (location = 1) out vec3 outNormal;

void main()
{
  outLambert = texture(sampLambert, inUV).rgb;
  outNormal = (texture(sampNormal, inUV).xyz * 2.0 - 1.0) * inNormalMat;
}
