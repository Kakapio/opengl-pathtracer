#include "Camera.hpp"
#include "glm/geometric.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

void Camera::LookAt(glm::vec3 lookAt, glm::vec3 up) {
  shouldLookAt = true;
  this->lookAt = lookAt;
  this->up = glm::normalize(up);
}

glm::mat4 Camera::GetModelviewMatrix(glm::mat4 mv, float aspectRatio, float fov) {
  const float cameraSize = 10;
  
  mv *= glm::translate(glm::mat4(1.0), pos);
	
  if (shouldLookAt) {
    glm::vec3 lookDir = glm::normalize(lookAt - pos);
    lookDir = -lookDir;
    mv *= glm::inverse(glm::lookAt(glm::vec3(0,0,0), lookDir, up));
  } else {
    mv *= glm::mat4_cast(rot);
  }

  float tan = tanf(fov/2.0f) * 2;
  mv *= glm::scale(glm::mat4(1.0), glm::vec3(aspectRatio * tan, tan, 1) * cameraSize);

  return mv;
}

glm::mat4 Camera::GetViewMatrix() {
  if (!shouldLookAt) {
    lookAt = {0.0,0.0,1.0};
    lookAt = rot * lookAt;
    lookAt += pos;
  }

  return glm::lookAt(pos, lookAt, up);
}

