#version 300 es

precision highp float;

layout (location = 0) out vec3 outColor;

uniform sampler2D   sampLambert;
uniform sampler2D   sampNormal;
uniform sampler2D   sampPBRMaps;
uniform sampler2D   sampDepth;
uniform samplerCube sampShadow;

uniform vec3  lightPos;
uniform vec3  lightColor;
uniform float lightBrightness;
uniform vec2  screenSize;
uniform mat4  matView;
uniform float farPlane;

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

float calcShadow(vec3 worldPos, vec3 worldNormal)
{
  vec3 fragToLight = worldPos - lightPos;
  float closestDepth = texture(sampShadow, fragToLight).r * farPlane;
  float currentDepth = length(fragToLight);

  float bias = max(0.05 * (1.0 - dot(worldNormal, fragToLight)), 0.005);
  return closestDepth > currentDepth - bias ? 1.0 : 0.0;
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
  float shadow = calcShadow(worldPos, worldNormal);
  vec3 radiance = lightBrightness * lightColor * cosTheta * attenuation * shadow;

  outColor = vec3(0.0);
  if(depth < 1.0)
  {
    outColor = radiance * lambert;
  }
  gl_FragDepth = depth;
}
