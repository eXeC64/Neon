#version 300 es

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec2 vertexUV;
layout (location = 2) in vec3 vertexNorm;

out vec2 inUV;
out mat3 inNormalMat;

uniform mat4 matPos;
uniform mat4 matView;

void main()
{
  inUV = vertexUV;
  gl_Position = matView * matPos * vec4(vertexPos, 1);

  // Create a matrix for converting from the polygon's tangent
  // space to world space
  mat3 normMat = mat3(matPos);
  vec3 normal = normalize(normMat * vertexNorm);
  vec3 tangent = normalize(normMat[0]);
  vec3 binormal = normalize(normMat[1]);
  inNormalMat = mat3(
      tangent.x, binormal.x, normal.x,
      tangent.y, binormal.y, normal.y,
      tangent.z, binormal.z, normal.z
  );
}
