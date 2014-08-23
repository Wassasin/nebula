#pragma once

#include "util/glm_include.hpp"

struct tri
{
	glm::vec3 a, b, c;

	tri(glm::vec3 a, glm::vec3 b, glm::vec3 c)
	: a(a)
	, b(b)
	, c(c)
	{}
};
