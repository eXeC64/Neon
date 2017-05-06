#version 300 es

precision highp float;

in vec4 fragPos;

uniform vec3 lightPos;
uniform float farPlane;

void main()
{
  float lightDist = length(fragPos.xyz - lightPos) / farPlane;
  gl_FragDepth = lightDist;
}
