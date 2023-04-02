#ifndef __CAMERA_HPP__
#define __CAMERA_HPP__

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera {
protected:
  glm::vec3 pos{0.0,0.0,0.0};
  
  bool shouldLookAt = false;
  glm::vec3 lookAt, up{0.0,1.0,0.0};

  glm::quat rot{glm::vec3(0.0,glm::radians(180.0),0.0)};

  void LookAt(glm::vec3 lookAt, glm::vec3 up = {0.0,1.0,0.0});

public:
  virtual void Update(float deltaTime) = 0;

  glm::mat4 GetViewMatrix();
  glm::mat4 GetModelviewMatrix(glm::mat4 mv, float aspectRatio, float fov);

};

#endif
