#version 300 es

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec2 vertexUV;
layout (location = 2) in vec3 vertexNorm;

uniform mat4 matPos;

void main()
{
  gl_Position = matPos * vec4(vertexPos, 1);
}
