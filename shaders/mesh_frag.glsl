#version 300 es

precision highp float;

uniform sampler2D sampDiffuse;
uniform sampler2D sampNormal;

uniform mat4 matPos;
uniform mat4 matView;

mat4 invMatPos;
mat4 invMatView;

in vec2 inUV;
in vec3 inNormal;
in vec3 inWorldPos;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 outNormal;

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}


void main()
{
  invMatPos = inverse(matPos);
  invMatView = inverse(matView);
  outColor = texture(sampDiffuse, inUV).rgb;
  vec3 normMap = texture(sampNormal, inUV).xyz;
  float rotX = dot(vec3(1,0,0), inNormal);
  float rotY = dot(vec3(0,1,0), inNormal);
  float rotZ = dot(vec3(0,0,1), inNormal);
  vec3 rotNorm = normMap;
  rotNorm = vec3(rotationMatrix(cross(vec3(1,0,0), rotNorm), rotX) * vec4(rotNorm,1));
  rotNorm = vec3(rotationMatrix(cross(vec3(0,1,0), rotNorm), rotY) * vec4(rotNorm,1));
  rotNorm = vec3(rotationMatrix(cross(vec3(0,0,1), rotNorm), rotZ) * vec4(rotNorm,1));
  outNormal = rotNorm;
}
