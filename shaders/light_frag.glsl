#version 300 es

precision mediump float;

layout (location = 0) out vec3 outColor;

uniform sampler2D sampDiffuse;
uniform sampler2D sampNormal;
uniform sampler2D sampDepth;

uniform float time;
uniform vec2 screenSize;
uniform mat4 matView;

vec2 screenPos = gl_FragCoord.xy / screenSize;

float normalizeDepth(float value, float min, float max)
{
  float Q = max / (max - min);
  float invFar = 1.0 / max;
  return invFar / (Q - value);
}

vec3 worldPos()
{
 float z = texture(sampDepth, screenPos).x;
 vec4 sPos = vec4(screenPos * 2.0 - 1.0, z * 2.0 - 1.0, 1.0);
 sPos = inverse(matView) * sPos;
 return sPos.xyz / sPos.w;
}

void main()
{
  vec3 diffuse = texture(sampDiffuse, screenPos).rgb;
  vec3 normal = texture(sampNormal, screenPos).rgb;
  float depth = texture(sampDepth, screenPos).x;
  outColor = diffuse;
}
