#ifndef __RAY3D_HPP__
#define __RAY3D_HPP__

#include <glm/glm.hpp>

struct Ray3D {
  glm::vec3 start;
  glm::vec3 direction;

  Ray3D(glm::vec3 start, glm::vec3 direction): start(start), direction(direction) { }
  Ray3D(const Ray3D &other) : start(other.start), direction(other.direction) { }

private:
  Ray3D();
};

#endif
