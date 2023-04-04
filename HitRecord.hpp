#ifndef __HITRECORD__
#define __HITRECORD__

#include <glm/glm.hpp>
#include "Material.h"
#include "TextureImage.h"
#define _USE_MATH_DEFINES
#include <cmath>

const float MaxFloat = MAXFLOAT;

struct HitRecord {
    float time = MaxFloat;
    glm::vec3 intersection;
    glm::vec3 normal;
    util::Material* mat = NULL;
    glm::vec2 texCoord;
    util::TextureImage* texture = NULL;

private:
};

#endif
