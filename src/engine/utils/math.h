#pragma once
#include <glm/glm.hpp>

namespace engine::utils {

struct Rect
{
    glm::vec2 position;
    glm::vec2 size;
};

struct FColor
{
    float r;
    float g;
    float b;
    float a;
};

} // namespace engine::utils