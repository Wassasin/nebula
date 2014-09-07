#pragma once

#include "gl/glm_include.hpp"

struct tri
{
	glm::vec3 a, b, c;

	tri(glm::vec3 a, glm::vec3 b, glm::vec3 c)
	: a(a)
	, b(b)
	, c(c)
	{}

	glm::vec3 centroid() const
	{
		return (a + b + c) / 3.0f;
	}

	tri operator*(GLfloat rhs)
	{
		glm::vec3 m = centroid();

		return tri(
			m + (a - m) * rhs,
			m + (b - m) * rhs,
			m + (c - m) * rhs
		);
	}
};
