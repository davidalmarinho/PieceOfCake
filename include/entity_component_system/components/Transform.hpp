#pragma once

#include <glm/glm.hpp>

#include "ECS.hpp"

class Transform : public Component
{
private:
  glm::vec3 position;
  glm::vec3 scaleVec = glm::vec3{1.0f};
  glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::mat3 normalMatrix = glm::mat3{1.0f};

public:  
  Transform(glm::vec3 position);
  Transform();
  
  void build(glm::vec3 position);
  void update(float deltaTime) override;

  void translate(glm::vec3 translation);
  void translate(float x, float y, float z);
  void rotate(float xAngle, float yAngle, float zAngle);
  void scale(float x, float y, float z);
  void scale(glm::vec3 scale);

  glm::mat4 getTranslationMatrix();
  glm::mat4 getRotationMatrix();
  glm::mat4 getScaleMatrix();
  glm::mat3 getNormalMatrix();
  glm::mat4 getModelMatrix();

  // Encapsulation.
  const glm::vec3 getPosition();
  void setPosition(float x, float y, float z);
  const glm::vec3 getScale();
  void setScale(float x, float y, float z);
  const glm::vec3 getRotation();
  void setRotation(float xAngleDegrees, float yAngleDegrees, float zAngleDegrees);
};