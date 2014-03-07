#include "rendercontext.hpp"

#include <GL/gl.h>
#include <glm/gtc/matrix_transform.hpp>

rendercontext::rendercontext()
: m_cbs()
, m_size(1024, 768)
, mvp(glm::perspective(60.0f, (GLfloat)m_size.first/(GLfloat)m_size.second, 1.0f, 100.0f) * glm::lookAt(
	glm::vec3(2.5, 0.5, 2.5),
	glm::vec3(0.0, 0.0, 0.0),
	glm::vec3(0.0, 1.0, 0.0)
))
{}

const std::pair<size_t, size_t>& rendercontext::size() const
{
	return m_size;
}

void rendercontext::add_cb(rcphase p, const callback_t &f)
{
	m_cbs[p].push_back(f);
}
