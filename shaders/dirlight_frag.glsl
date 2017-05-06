#version 300 es

precision highp float;

layout (location = 0) out vec3 outColor;

uniform sampler2D sampLambert;
uniform sampler2D sampNormal;
uniform sampler2D sampPBRMaps;
uniform sampler2D sampDepth;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform float lightBrightness;
uniform vec2 screenSize;

void main()
{
  vec2 screenPos = gl_FragCoord.xy / screenSize;
  vec3 lambert = texture(sampLambert, screenPos).rgb;
  vec3 worldNormal = texture(sampNormal, screenPos).xyz;
  float depth = texture(sampDepth, screenPos).x;

  float cosTheta = max(dot(normalize(lightDir), worldNormal), 0.0);
  vec3 radiance = lightBrightness * lightColor * cosTheta;

  outColor = vec3(0.0);
  if(depth < 1.0)
  {
    outColor = radiance * lambert;
  }
  gl_FragDepth = depth;
}
