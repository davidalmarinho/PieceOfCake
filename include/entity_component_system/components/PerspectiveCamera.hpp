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
  glm::vec3 targetPosition = glm::vec3{0.0f};
  glm::mat4 projectionMatrix{1.0f};
  glm::mat4 viewMatrix{1.0f};
  float foV = 60.0f; // Fied of View in degres.
  float aspectRatio = 800.0f / 600.0f;

  void moveCamera(float deltaTime);
  void adjustDirection(float deltaTime);

public:
  // For translation.
  glm::vec3 position = glm::vec3(4.0f, 2.0f, 2.0f);
  // glm::vec3 position = glm::vec3(4.0f, 2.0f, 2.0f);
  float zNear = 0.1f, zFar = 10.0f;

  // Euler rotations.
  float pitch = -30.0f;
  float yaw = 180.0f;

  void update(float deltaTime) override;

  const glm::mat4& getViewMatrix();
  const glm::mat4& getProjection();

  const float getFoV();
  void setFoV(float foV);
};