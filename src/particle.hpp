#pragma once

#include "gl/glm_include.hpp"
#include "gl/glm_msgpack.hpp"

struct particle_t
{
	glm::vec3 pos;
	glm::vec4 color;

	MSGPACK_DEFINE(pos, color)
};
