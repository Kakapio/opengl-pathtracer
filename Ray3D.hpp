#ifndef __RAY3D_HPP__
#define __RAY3D_HPP__

#include <glm/glm.hpp>

struct Ray3D {
  glm::vec4 start;
  glm::vec4 direction;

  Ray3D(glm::vec3 start, glm::vec3 direction): start(start, 1), direction(direction, 0) { }
  Ray3D(const Ray3D &other) : start(other.start), direction(other.direction) { }

private:
  Ray3D();
};

#endif
