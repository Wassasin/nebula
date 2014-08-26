#pragma once

#include "gl/glm_include.hpp"
#include "gl/glm_msgpack.hpp"

struct particle_t
{
	glm::vec3 pos;
	glm::uvec4 color;
	GLfloat z;

	MSGPACK_DEFINE(pos, color, z)
};
