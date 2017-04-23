#version 300 es

layout (location = 0) in vec3 vertexPos;

uniform mat4 matPos;
uniform mat4 matView;

void main()
{
  gl_Position = matView * matPos * vec4(vertexPos, 1);
}
