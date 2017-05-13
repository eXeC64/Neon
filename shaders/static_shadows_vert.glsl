#version 300 es

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec2 vertexUV;
layout (location = 2) in vec3 vertexNorm;

uniform mat4 matPos;
uniform mat4 matLightProj;

void main()
{
  gl_Position = matLightProj * matPos * vec4(vertexPos, 1);
}
