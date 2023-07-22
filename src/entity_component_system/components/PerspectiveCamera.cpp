#include "PerspectiveCamera.hpp"

#include "KeyListener.hpp"

void PerspectiveCamera::update(float deltaTime)
{
  bool goForward = KeyListener::isKeyPressed(GLFW_KEY_W);
  bool goBack    = KeyListener::isKeyPressed(GLFW_KEY_S);
  bool goRight   = KeyListener::isKeyPressed(GLFW_KEY_D);
  bool goLeft    = KeyListener::isKeyPressed(GLFW_KEY_A);

  if (!goForward && !goBack && !goRight && !goLeft) return;

  float moveSpd = 2.0f;
  float xVec = this->position.x - 0; // TODO: Those 0 should be from targetVec
  float yVec = this->position.y - 0;
  float zVec = this->position.z - 0;

  // Move around scenario.
  if (goForward || goBack) {
    glm::vec3 translateVec = glm::normalize(glm::vec3(xVec, yVec, zVec));
    translateVec *= deltaTime * moveSpd;

    if (goForward) {
      this->position -= translateVec;
    }
    else if (goBack) {
      this->position += translateVec;
    } 
  }

  if (goRight || goLeft) {
    glm::vec3 translateVec = glm::normalize(glm::vec3(xVec, yVec, zVec));

    // Rotate vector by 90º degrees in xx and yy axis.
    glm::mat4 rotationMat(1);
    rotationMat = glm::rotate(rotationMat, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
    translateVec = glm::vec3(rotationMat * glm::vec4(translateVec.x, translateVec.y, 0.0f, 1.0) * moveSpd * deltaTime);

    if (goRight) {
      this->position += translateVec;
    }
    else if (goLeft) {
      this->position -= translateVec;
    }
  }
}

const glm::mat4& PerspectiveCamera::getViewMatrix()
{
  this->viewMatrix = glm::lookAt(
    this->position,              // Camera position.
    glm::vec3(0.0f, 0.0f, 0.0f), // Camera looking at.
    glm::vec3(0.0f, 0.0f, 1.0f)  // Up vector.
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