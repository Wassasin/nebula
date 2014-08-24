#pragma once

#include "glm_include.hpp"

static inline void set_rgb(glm::uvec4& x, const glm::uvec3 color)
{
	x.r = color.r;
	x.g = color.g;
	x.b = color.b;
}

static inline glm::vec3 downcast(const glm::uvec3 p, uint8_t fX = 255, uint8_t fY = 255, uint8_t fZ = 255)
{
	return glm::vec3((GLfloat)p.x / fX, (GLfloat)p.y / fY, (GLfloat)p.z / fZ);
}

static inline glm::uvec3 upcast(const glm::vec3 p, uint8_t fX = 255, uint8_t fY = 255, uint8_t fZ = 255)
{
	glm::vec3 tmp = glm::round(p * glm::vec3(fX, fY, fZ));
	return glm::clamp(glm::uvec3(tmp.x, tmp.y, tmp.z), glm::uvec3(0), glm::uvec3(255));
}

static inline glm::ivec3 iupcast(const glm::vec3 p, uint8_t fX = 255, uint8_t fY = 255, uint8_t fZ = 255)
{
	glm::vec3 tmp = glm::round(p * glm::vec3(fX, fY, fZ));
	return glm::clamp(glm::ivec3(tmp.x, tmp.y, tmp.z), glm::ivec3(-255), glm::ivec3(255));
}
