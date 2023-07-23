#pragma once

#include <glm/glm.hpp>

#include "ECS.hpp"

class Transform : public Component
{
public:
  glm::vec3 position;
  glm::vec3 rotation;
  
  // For rotation. Quaternions.
  // glm::mat4 quat;
  // float rotAngle = 0.0f; // In degrees.
  // glm::vec3 rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);

  // For scaling.
  // glm::mat4 scalingMatrix = ;
  
  Transform(glm::vec3 position);
  Transform();
  
  void build(glm::vec3 position);
};