#pragma once

#include "gl/glm_msgpack.hpp"

struct star_t
{
	glm::vec3 pos;
	glm::vec3 color;

	MSGPACK_DEFINE(pos, color)
};
