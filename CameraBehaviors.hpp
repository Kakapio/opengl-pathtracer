#ifndef __CAMERA_BEHAVIORS_HPP__
#define __CAMERA_BEHAVIORS_HPP__

#include "Camera.hpp"

class StationaryCamera : public Camera {
  
public:
  StationaryCamera(glm::vec3 position);
  
  virtual void Update(float deltaTime) override;

  void LookAt(glm::vec3 lookAt, glm::vec3 up = {0.0,1.0,0.0});
};

class HelicopterCamera : public Camera {
  float currentAngle;
  float radius;
  float speed;

public:
  HelicopterCamera(float height, float radius, float speed, float focusHeight = 0);

  virtual void Update(float deltaTime) override;

};

class FirstPersonCamera : public Camera {
  float movementSpeed;
  float rotationSpeed;

  int input = 0;

  glm::vec3 eulerAngles;

  glm::vec3 startPos, startEulers;

public:
  FirstPersonCamera(glm::vec3 position, float movementSpeed, float rotationSpeed);

  virtual void Update(float deltaTime) override;

  enum class Input : int {
    MoveForward = 1 << 0,
    MoveBackward = 1 << 1,
    LookUp = 1 << 2,
    LookDown = 1 << 3,
    LookLeft = 1 << 4,
    LookRight = 1 << 5
  };

  void PressedButton(Input button);
  void ReleasedButton(Input button);

  void Reset();

};


#endif
