#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
  mat4 model;
  mat4 view;
  mat4 proj;
  mat4 normalMatrix;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoords;
layout(location = 3) in vec3 inNormalCoords;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoords;

// TODO: In future is good idea to upload this variables from a GUI interface.
const vec3 DIRECTION_TO_LIGHT = normalize(vec3(-1.0, -3.0, -1.0));
const float AMBIENT = 0.2;

void main() {
  gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
  // gl_Position = vec4(inPosition, 0.0, 1.0);
  
  vec3 normalWorldSpace = normalize(mat3(ubo.normalMatrix) * inNormalCoords);

  float lightIntensity = AMBIENT + max(dot(normalWorldSpace, DIRECTION_TO_LIGHT), 0);
  fragColor = inColor * lightIntensity;
  fragTexCoords = inTexCoords;
}
