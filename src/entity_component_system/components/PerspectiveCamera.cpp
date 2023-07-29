#include "PerspectiveCamera.hpp"

#include "KeyListener.hpp"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

void PerspectiveCamera::moveCamera(float deltaTime)
{
  bool goForward  = KeyListener::isKeyPressed(GLFW_KEY_W);
  bool goBackward = KeyListener::isKeyPressed(GLFW_KEY_S);
  bool goRight    = KeyListener::isKeyPressed(GLFW_KEY_D);
  bool goLeft     = KeyListener::isKeyPressed(GLFW_KEY_A);

  float moveSpd = 2.0f;

  // Move around scenario.
  if (goForward) {
    this->position += moveSpd * deltaTime * targetPosition;
  }
  else if (goBackward) {
    this->position -= moveSpd * deltaTime * targetPosition;
  }

  if (goRight) {
    this->position += moveSpd * deltaTime * glm::normalize(glm::cross(targetPosition, glm::vec3(0.0f, 0.0f, 1.0f)));
  }
  else if (goLeft) {
    this->position -= moveSpd * deltaTime * glm::normalize(glm::cross(targetPosition, glm::vec3(0.0f, 0.0f, 1.0f)));
  }
}

void PerspectiveCamera::adjustDirection(float deltaTime)
{
  bool up    = KeyListener::isKeyPressed(GLFW_KEY_UP);
  bool down  = KeyListener::isKeyPressed(GLFW_KEY_DOWN);
  bool right = KeyListener::isKeyPressed(GLFW_KEY_RIGHT);
  bool left  = KeyListener::isKeyPressed(GLFW_KEY_LEFT);

  float moveSpd = 60.0f;

  if (up) {
    this->pitch += moveSpd * deltaTime;
  }
  else if (down) {
    this->pitch -= moveSpd * deltaTime;
  }

  if (left) {
    this->yaw += moveSpd * deltaTime;
  }
  else if (right) {
    this->yaw -= moveSpd * deltaTime;
  }

  if(pitch > 89.0f)
    pitch =  89.0f;

  if(pitch < -89.0f)
    pitch = -89.0f;

  glm::vec3 direction;
  direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  direction.y = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  direction.z = sin(glm::radians(pitch));

  this->targetPosition = glm::normalize(direction);
}

void PerspectiveCamera::update(float deltaTime)
{
  this->moveCamera(deltaTime);
  this->adjustDirection(deltaTime);
}

const glm::mat4& PerspectiveCamera::getViewMatrix()
{
  this->viewMatrix = glm::lookAt(
    this->position,                        // Camera position.
    this->position + this->targetPosition, // Camera looking at.
    glm::vec3(0.0f, 0.0f, 1.0f)            // Up vector.
  );

  return this->viewMatrix;
}

const glm::mat4& PerspectiveCamera::getProjection()
{
  this->projectionMatrix = glm::perspective(
    glm::radians(foV), // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
    this->aspectRatio, // Depends on the size of your window. Notice that 4/3 == 800/600
    this->zNear,       // Near clipping plane. Keep as big as possible.
    this->zFar         // Far clipping plane. Keep as little as possible.
  );

  return this->projectionMatrix;
}

const float PerspectiveCamera::getFoV()
{
  return this->foV;
}

void PerspectiveCamera::setFoV(float foV)
{
  if (foV >= 30 && foV <= 90) {
    this->foV = foV;
  }
  else {
    // TODO: Throw warning / error.
  }
}