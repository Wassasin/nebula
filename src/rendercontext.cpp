#include "rendercontext.hpp"

#include <iostream>

#include <GL/gl.h>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

rendercontext::rendercontext()
: m_cbs()
, m_size(500, 500)
, camera({glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)})
{}

const std::pair<size_t, size_t>& rendercontext::size() const
{
	return m_size;
}

void rendercontext::add_cb(rcphase p, const callback_t &f)
{
	m_cbs[p].push_back(f);
}

glm::mat4 camera_t::to_matrix() const
{
	glm::vec3 direction(
		glm::cos(rotation.y) * glm::sin(rotation.x),
		glm::sin(rotation.y),
		glm::cos(rotation.y) * glm::cos(rotation.x)
	);

	return glm::lookAt(
		position,
		position + direction,
		glm::vec3(0.0, 1.0, 0.0)
	);
}
