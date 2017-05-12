#version 300 es

const int MAX_BONES = 32;

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec3 vertexNorm;
layout (location = 2) in vec2 vertexUV;
layout (location = 3) in vec4 boneWeights;
layout (location = 4) in vec4 boneIds;

out vec2 inUV;
out mat3 inNormalMat;

uniform mat4 matPos;
uniform mat4 matView;
uniform mat4 boneTransforms[MAX_BONES];

void main()
{
  inUV = vertexUV;

  vec4 localPos = vec4(0.0);
  vec4 localNormal = vec4(0.0);

  for(int i = 0; i < 4; ++i)
  {
    vec4 pos = boneTransforms[int(boneIds[i])] * vec4(vertexPos, 1);
    localPos += pos * boneWeights[i];

    vec4 norm = boneTransforms[int(boneIds[i])] * vec4(vertexNorm, 0);
    localNormal += norm * boneWeights[i];
  }

  gl_Position = matView * matPos * localPos;

  // Create a matrix for converting from the polygon's tangent
  // space to world space
  mat3 normMat = mat3(matPos);
  vec3 normal = normalize((matPos * localNormal).xyz);
  vec3 tangent = normalize(normMat[0]);
  vec3 binormal = normalize(normMat[1]);
  inNormalMat = mat3(
      tangent.x, binormal.x, normal.x,
      tangent.y, binormal.y, normal.y,
      tangent.z, binormal.z, normal.z
  );
}
