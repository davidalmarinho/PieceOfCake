#include "Transform.hpp"

Transform::Transform(glm::vec3 position)
{
  this->build(position);
}

Transform::Transform()
{
  this->build(glm::vec3(0, 0, 0));
}

void Transform::build(glm::vec3 position)
{
  this->position = position;
}