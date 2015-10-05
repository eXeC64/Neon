#version 300 es

precision mediump float;

layout (location = 0) out vec3 outColor;

uniform sampler2D sampDiffuse;
uniform sampler2D sampNormal;
uniform sampler2D sampDepth;

uniform float time;
uniform vec3 viewPos;
uniform vec2 screenSize;
uniform mat4 matView;

mat4 invMatView = inverse(matView);

vec2 screenPos = gl_FragCoord.xy / screenSize;

float normalizeDepth(float value, float min, float max)
{
  float Q = max / (max - min);
  float invFar = 1.0 / max;
  return invFar / (Q - value);
}

vec3 calcWorldPos()
{
 float z = texture(sampDepth, screenPos).x;
 vec4 sPos = vec4(screenPos * 2.0 - 1.0, z * 2.0 - 1.0, 1.0);
 sPos = invMatView * sPos;
 return sPos.xyz / sPos.w;
}

void main()
{
  vec3 diffuse = texture(sampDiffuse, screenPos).rgb;
  vec3 worldNormal = normalize(texture(sampNormal, screenPos).xyz);
  vec3 worldPos = calcWorldPos();
  float depth = texture(sampDepth, screenPos).x;

  vec3 lightPos = 3.0 * vec3(sin(2.0*time),1.0,cos(2.0*time));
  vec3 lightDir = normalize(lightPos - worldPos);
  float light = clamp(dot(lightDir,worldNormal), 0.15, 1.0);

  outColor = vec3(0.5);
  if(depth < 1.0)
  {
    outColor = light * diffuse;
  }
}
