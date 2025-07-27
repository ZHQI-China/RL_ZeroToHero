#pragma once
#include <glm/glm.hpp>
#include <cmath>

using vec2i = glm::ivec2;
using vec2f = glm::vec2;
using vec2d = glm::dvec2;
//#define  dot  glm::dot

static float Dot(const vec2f& a, const vec2f& b) {
	return a.x * b.x + a.y * b.y;
}

static constexpr float PI = 3.141592654f;

using vec3f = glm::fvec3;
//using distance = glm::distance;

