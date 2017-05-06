#version 300 es

precision highp float;

layout (location = 0) out vec3 outColor;

uniform sampler2D sampLambert;
uniform sampler2D sampNormal;
uniform sampler2D sampPBRMaps;
uniform sampler2D sampDepth;
uniform sampler2D sampShadow;

uniform vec3 lightPos;
uniform vec3 lightDir;
uniform float innerAngle;
uniform float outerAngle;
uniform vec3 lightColor;
uniform float lightBrightness;
uniform vec2 screenSize;
uniform mat4 matView;
uniform mat4 matLight;
uniform float nearPlane;
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
  return 1.0 / d;
}

float transformDepth(float depth)
{
  float z = depth * 2.0 - 1.0;
  return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

float calcShadow(vec3 worldPos)
{
  vec4 lightSpacePos = matLight * vec4(worldPos, 1.0);
  vec3 projCoords = (lightSpacePos.xyz / lightSpacePos.w) * 0.5 + 0.5;
  float closestDepth = texture(sampShadow, projCoords.xy).r;
  float currentDepth = projCoords.z;
  float bias = 0.0001;
  return closestDepth > currentDepth - bias ? 1.0 : 0.0;
}

void main()
{
  outColor = vec3(0.0);
  vec2 screenPos = gl_FragCoord.xy / screenSize;
  float depth = texture(sampDepth, screenPos).x;
  gl_FragDepth = depth;
  vec3 worldPos = calcWorldPos(screenPos);

  if(depth < 1.0)
  {
    vec3 lambert = texture(sampLambert, screenPos).rgb;
    vec3 worldNormal = texture(sampNormal, screenPos).xyz;

    vec3 fragToLight = normalize(lightPos - worldPos);
    float dirTheta = dot(lightDir, normalize(-fragToLight));

    float attenuation = calcAttenuation(worldPos, lightPos);
    float penumbra = smoothstep(outerAngle, innerAngle, dirTheta);
    float shadow = calcShadow(worldPos);

    float cosTheta = max(dot(worldNormal, fragToLight), 0.0);
    vec3 radiance = lightBrightness * lightColor * cosTheta * attenuation * penumbra * shadow;
    outColor = radiance * lambert;
  }
}
