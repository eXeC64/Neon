#version 300 es

layout (location = 0) in vec3 vertexPos;

uniform mat4 matLight;

//out vec4 inLightSpacePos;

void main()
{
  gl_Position = vec4(vertexPos, 1);
  //inLightSpacePos = matLight * vec4(vertexPos, 1.0);
}
