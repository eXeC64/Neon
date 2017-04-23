#version 300 es

precision highp float;

layout (location = 0) out vec3 outColor;

uniform vec3 color;

void main()
{
  outColor = color;
}
