#version 300 es

precision highp float;

layout (location = 0) out vec3 outColor;

uniform sampler2D sampBuffer;
uniform sampler2D sampDepth;

uniform vec2 screenSize;
uniform float gamma;
uniform float exposure;

vec3 ACESFilm(vec3 x)
{
  float a = 2.51f;
  float b = 0.03f;
  float c = 2.43f;
  float d = 0.59f;
  float e = 0.14f;
  return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

vec3 gammaCorrect(float gamma, vec3 color)
{
  return pow(color, vec3(1.0/gamma));
}

void main()
{
  vec2 screenPos = gl_FragCoord.xy / screenSize;
  vec3 hdrColor = texture(sampBuffer, screenPos).rgb + vec3(0.01);
  outColor = gammaCorrect(gamma, ACESFilm(hdrColor * pow(2.0, exposure)));
  gl_FragDepth = texture(sampDepth, screenPos).r;
}
