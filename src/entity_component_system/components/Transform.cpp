#include "Transform.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

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

void Transform::update(float deltaTime)
{
  // this->entity->transform = this;
}

void Transform::translate(glm::vec3 translation)
{
  this->position += translation;
}

void Transform::translate(float x, float y, float z)
{
  this->position.x += x;
  this->position.y += y;
  this->position.z += z;
}

void Transform::rotate(float xDegreeAngle, float yDegreeAngle, float zDegreeAngle) 
{
  if (this->rotation.x + xDegreeAngle > 360) {
    this->rotation.x += xDegreeAngle - 360;
  }
  else if (this->rotation.x + xDegreeAngle < -360) {
    this->rotation.x += 360 + xDegreeAngle;
  }
  else {
    this->rotation.x += xDegreeAngle;
  }

  if (this->rotation.y + yDegreeAngle > 360) {
    this->rotation.y += yDegreeAngle - 360;
  }
  else if (this->rotation.y + yDegreeAngle < -360) {
    this->rotation.y += 360 + yDegreeAngle;
  }
  else {
    this->rotation.y += xDegreeAngle;
  }

  if (this->rotation.z + zDegreeAngle > 360) {
    this->rotation.z += zDegreeAngle - 360;
  }
  else if (this->rotation.z + zDegreeAngle < -360) {
    this->rotation.z += 360 + zDegreeAngle;
  }
  else {
    this->rotation.z += xDegreeAngle;
  }
}

void Transform::scale(glm::vec3 scale)
{
  this->scaleVec += scale;
}

void Transform::scale(float x, float y, float z)
{
  this->scaleVec.x += x;
  this->scaleVec.y += y;
  this->scaleVec.z += z;
}

glm::mat4 Transform::getModelMatrix()
{
  return getTranslationMatrix() * getScaleMatrix() * getRotationMatrix();
}

glm::mat3 Transform::getNormalMatrix()
{
  glm::mat4 modelMatrix = getModelMatrix();
  return glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
}

glm::mat4 Transform::getTranslationMatrix()
{
  glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
  return translationMatrix;
}

glm::mat4 Transform::getScaleMatrix()
{
  glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scaleVec);
  return scaleMatrix;
}

glm::mat4 Transform::getRotationMatrix()
{
  // Conversion from Euler angles (in radians) to Quaternion
  glm::quat rotQuat = glm::quat(glm::radians(this->rotation));
  glm::mat4 rotationMatrix = glm::mat4_cast(rotQuat);

  return rotationMatrix;
}

const glm::vec3 Transform::getPosition()
{
  return this->position;
}

void Transform::setPosition(float x, float y, float z)
{
  this->position.x = x;
  this->position.y = y;
  this->position.z = z;
}

const glm::vec3 Transform::getScale()
{
  return this->scaleVec;
}

void Transform::setScale(float x, float y, float z)
{
  this->scaleVec.x = x;
  this->scaleVec.y = y;
  this->scaleVec.z = z;
}

const glm::vec3 Transform::getRotation()
{
  return this->rotation;
}

void Transform::setRotation(float xAngleDegrees, float yAngleDegrees, float zAngleDegrees)
{
  this->rotation.x = xAngleDegrees;
  this->rotation.y = yAngleDegrees;
  this->rotation.z = zAngleDegrees;
}