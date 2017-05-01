#version 300 es

precision highp float;

layout (location = 0) out vec3 outColor;

uniform sampler2D sampBuffer;

uniform vec2 screenSize;
uniform float gamma;

vec3 gammaCorrect(float gamma, vec3 color)
{
  return pow(color, vec3(1.0/gamma));
}

void main()
{
  vec2 screenPos = gl_FragCoord.xy / screenSize;
  vec3 color = texture(sampBuffer, screenPos).rgb;
  outColor = gammaCorrect(gamma, color);
}
