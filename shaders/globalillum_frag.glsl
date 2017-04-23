#version 300 es

precision highp float;

layout (location = 0) out vec3 outColor;

uniform sampler2D sampColor;

uniform vec3 lightColor;
uniform vec2 screenSize;

void main()
{
  vec2 screenPos = gl_FragCoord.xy / screenSize;
  vec3 diffuse = texture(sampColor, screenPos).rgb;
  outColor = lightColor * diffuse;
}
