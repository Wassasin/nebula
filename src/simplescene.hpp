#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <boost/optional.hpp>

#include <cmath>

#include "rendercontext.hpp"
#include "shader.hpp"
#include "vao.hpp"
#include "vbo.hpp"
#include "defines.hpp"

class simplescene
{
private:
	struct state_t {
		shader_uniform_setter<glm::mat4> mvp;
		qi i;
	};

	shader_program m_program;
	vao m_va;
	vbo<GLfloat> m_vb;

	boost::optional<state_t> m_state;

	static qfloat in_range(qi i, qi steps, qfloat from, qfloat to)
	{
		qfloat range = to - from;
		qfloat step = range / (qfloat)steps;

		qi j = i % steps;

		return from + step * ((qfloat) j);
	}

	void init()
	{
		gl::clear_color(0.1f, 0.1f, 0.1f, 0.0f);

		m_program.attach(shader::from_file(shader_type::vertex, "shaders/simple.vertexshader"));
		m_program.attach(shader::from_file(shader_type::fragment, "shaders/simple.fragmentshader"));
		m_program.link();

		m_va.bind();

		m_vb.bind(GL_ARRAY_BUFFER);
		m_vb.data({
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			0.0f,  1.0f, 0.0f,
		}, GL_STATIC_DRAW);

		auto mvp = m_program.uniform<glm::mat4>("MVP");

		m_state.reset({
			mvp,
			0
		});
	}

	void update_camera(rendercontext& r)
	{
		constexpr GLfloat dist = 2.0;
		qfloat rad = in_range(m_state->i++, 360, 0, 2.0 * M_PI);

		glm::mat4 projection = glm::perspective(60.0f, (GLfloat)r.size().first/(GLfloat)r.size().second, 1.0f, 100.0f);

		r.mvp = projection * glm::lookAt(
			glm::vec3(dist*std::cos(rad), 0.5, dist*std::sin(rad)),
			glm::vec3(0.0, 0.0, 0.0),
			glm::vec3(0.0, 1.0, 0.0)
		);
	}

	void draw(rendercontext& r)
	{
		gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_program.use();

		gl::enable_vertex_attribute_array(0);
		m_vb.bind(GL_ARRAY_BUFFER);
		gl::vertex_attribute_pointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		gl::draw_arrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(0);

		m_state->mvp.set(r.mvp);
	}

public:
	simplescene(rendercontext& r)
	: m_program(false)
	, m_va(false)
	, m_vb(false)
	, m_state()
	{
		r.add_cb(rcphase::init, [&](rendercontext&) { init(); });
		r.add_cb(rcphase::update, [&](rendercontext& r) { update_camera(r); });
		r.add_cb(rcphase::draw, [&](rendercontext& r) { draw(r); });
		r.add_cb(rcphase::cleanup, [&](rendercontext& r) { m_program.destroy(); });
	}
};
