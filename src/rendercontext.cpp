#include "rendercontext.hpp"

#include <glm/gtc/matrix_transform.hpp>

rendercontext::rendercontext()
: m_cbs()
, projectionmat(glm::perspective(60.0f, 1.333f, 1.0f, 100.0f))
, viewmat(glm::lookAt(
	glm::vec3(2.5, 0.5, 2.5),
	glm::vec3(0.5, 0.5, 0.5),
	glm::vec3(0.0, 1.0, 0.0)
))
{}

void rendercontext::add_cb(rcphase p, const callback_t &f)
{
	m_cbs[p].push_back(f);
}
