#version 300 es

precision highp float;

layout (location = 0) out vec3 outColor;

uniform sampler2D sampLambert;
uniform sampler2D sampNormal;
uniform sampler2D sampPBRMaps;
uniform sampler2D sampDepth;

uniform vec3  lightPos;
uniform vec3  lightColor;
uniform float lightBrightness;
uniform vec2  screenSize;
uniform mat4  matView;

vec3 calcWorldPos(vec2 screenPos)
{
 float z = texture(sampDepth, screenPos).x;
 vec4 sPos = vec4(screenPos * 2.0 - 1.0, z * 2.0 - 1.0, 1.0);
 mat4 invMatView = inverse(matView);
 sPos = invMatView * sPos;
 return sPos.xyz / sPos.w;
}

float calcAttenuation(vec3 worldPos, vec3 lightPos)
{
  float d = distance(lightPos, worldPos);
  return 1.0 / (d * d);
}

void main()
{
  vec2 screenPos = gl_FragCoord.xy / screenSize;
  vec3 lambert = texture(sampLambert, screenPos).rgb;
  vec3 worldNormal = texture(sampNormal, screenPos).xyz;
  vec3 worldPos = calcWorldPos(screenPos);
  float depth = texture(sampDepth, screenPos).x;

  vec3 lightDir = normalize(lightPos - worldPos);
  float cosTheta = max(dot(worldNormal, lightDir), 0.0);
  float attenuation = calcAttenuation(worldPos, lightPos);
  vec3 radiance = lightBrightness * lightColor * cosTheta * attenuation;

  outColor = vec3(0.0);
  if(depth < 1.0)
  {
    outColor = radiance * lambert;
  }
  gl_FragDepth = depth;
}
