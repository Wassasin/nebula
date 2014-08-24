#pragma once

#include "gl/glm_msgpack.hpp"

struct star_t
{
	glm::uvec3 color;
	glm::vec3 pos;

	MSGPACK_DEFINE(color, pos)
};
