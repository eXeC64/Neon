#version 300 es

precision mediump float;

layout (location = 0) out vec3 outColor;

uniform sampler2D sampDiffuse;
uniform sampler2D sampNormal;
uniform sampler2D sampPosition;

uniform vec2 screenSize;

void main()
{
  vec2 texCoord = gl_FragCoord.xy / screenSize;
  vec3 diffuse = texture(sampDiffuse, texCoord).rgb;
  vec3 normal = texture(sampNormal, texCoord).rgb;
  vec3 position = texture(sampPosition, texCoord).rgb;
  vec3 light = vec3(0.5 * (normal.x + normal.y));
  outColor = diffuse * light;
}
