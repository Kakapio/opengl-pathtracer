#include "CameraBehaviors.hpp"
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

StationaryCamera::StationaryCamera(glm::vec3 position) {
  pos = position;
}

void StationaryCamera::Update(float deltaTime) { }

void StationaryCamera::LookAt(glm::vec3 lookAt, glm::vec3 up) {
  Camera::LookAt(lookAt,up);
}

HelicopterCamera::HelicopterCamera(float height, float radius, float speed, float focusHeight) {
  currentAngle = 0;
  this->radius = radius;
  this->speed = speed;

  pos.y = height;

  Update(0);
  
  LookAt(glm::vec3{0,focusHeight,0});
}

void HelicopterCamera::Update(float deltaTime) {
  currentAngle += deltaTime * speed;
  if (currentAngle > glm::radians(360.f)) {
    currentAngle -= glm::radians(360.f);
  }

  pos.x = sinf(currentAngle) * radius;
  pos.z = cosf(currentAngle) * radius;
}

FirstPersonCamera::FirstPersonCamera(glm::vec3 startPosition, float movementSpeed, float rotationSpeed):
  movementSpeed(movementSpeed), rotationSpeed(rotationSpeed) {
  pos = startPosition;
  eulerAngles = glm::eulerAngles(rot);

  startPos = pos;
  startEulers = eulerAngles;
}

const glm::vec3 RIGHT{1.0,0,0};
const glm::vec3 UP{0,1.0,0};
const glm::vec3 FORWARD{0,0,1.0};
void FirstPersonCamera::Update(float deltaTime) {

  if ((input & (int)Input::LookUp) != (input & (int)Input::LookDown)) {
    glm::vec3 axisAndDirection = (input & (int)Input::LookUp) ? -RIGHT : RIGHT;

    eulerAngles += rotationSpeed * deltaTime * axisAndDirection;
    eulerAngles.x = glm::clamp(eulerAngles.x, glm::radians(90.f), glm::radians(270.f));
  }

  if ((input & (int)Input::LookLeft) != (input & (int)Input::LookRight)) {
    glm::vec3 axisAndDirection = (input & (int)Input::LookLeft) ? -UP : UP;

    eulerAngles += rotationSpeed * deltaTime * axisAndDirection;
  }

  rot = glm::quat{eulerAngles};

  if ((input & (int)Input::MoveForward) != (input & (int)Input::MoveBackward)) {
    glm::vec3 direction = (input & (int)Input::MoveForward) ? FORWARD : -FORWARD;

    pos += movementSpeed * deltaTime * glm::rotate(rot, direction);
  }
}

void FirstPersonCamera::PressedButton(Input button) {
  input |= (int)button;
}

void FirstPersonCamera::ReleasedButton(Input button) {
  input &= ~(int)button;
}

void FirstPersonCamera::Reset() {
  pos = startPos;
  eulerAngles = startEulers;
}
