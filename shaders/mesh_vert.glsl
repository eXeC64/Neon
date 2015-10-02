#version 300 es

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec2 vertexUV;
layout (location = 2) in vec3 vertexNorm;

out vec2 inUV;
out vec3 inNormal;
out vec3 inWorldPos;

uniform mat4 matPos;
uniform mat4 matView;

void main()
{
  inUV = vertexUV;
  inNormal = (matView * matPos * vec4(vertexNorm, 1)).xyz;
  inWorldPos = (matPos * vec4(vertexPos, 1)).xyz;
  gl_Position = matView * matPos * vec4(vertexPos, 1);
}
