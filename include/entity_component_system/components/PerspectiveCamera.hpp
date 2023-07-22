#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "ECS.hpp"

class PerspectiveCamera : public Component
{
private:
  glm::mat4 projectionMatrix{1.0f};
  glm::mat4 viewMatrix{1.0f};
  float foV = 60.0f; // Fied of View in degres.
  float aspectRatio = 800.0f / 600.0f;
  float zNear = 0.1f, zFar = 10.0f;


public:
  // For translation.
  glm::vec3 position = glm::vec3(3.0f, 3.0f, 3.0f);

  // For rotation.
  float rotAngle = 0.0f; // In degrees.
  glm::vec3 rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);

  // For scaling.
  // glm::mat4 scalingMatrix = ;

  void update(float deltaTime) override;

  const glm::mat4& getViewMatrix();
  const glm::mat4& getProjection();

  const float getFoV();
  void setFoV(float foV);
};