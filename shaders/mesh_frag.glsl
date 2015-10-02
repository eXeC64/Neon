#version 300 es

precision mediump float;

uniform sampler2D sampDiffuse;
uniform sampler2D sampNormal;

in vec2 inUV;
in vec3 inNormal;
in vec3 inWorldPos;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outWorldPos;

void main()
{
  outColor = texture(sampDiffuse, inUV).rgb;
  outNormal = texture(sampNormal, inUV).rgb;
  outWorldPos = inWorldPos;
}
