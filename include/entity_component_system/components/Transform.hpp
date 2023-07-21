#pragma once

#include <glm/glm.hpp>

#include "ECS.hpp"

class Transform : public Component
{
public:
  glm::vec3 position;
  
  Transform(glm::vec3 position);
  Transform();
  
  void build(glm::vec3 position);
};