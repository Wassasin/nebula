#pragma once

#include <GL/glew.h>
#include <boost/optional.hpp>

#include "gl/rendercontext.hpp"
#include "gl/shader.hpp"
#include "gl/vao.hpp"
#include "gl/vbo.hpp"

#include "nebulagen.hpp"

class nebulascene
{
private:
	struct state_t
	{
		GLuint volume_texture;

		GLuint framebuffer;

		GLuint frontface_texture;
		GLuint final_texture;

		GLuint renderbuffer;
	};

	static void check_support();
	GLuint create_volumetexture();
	static GLuint create_2dtexture(const size_t width, const size_t height);
	static GLuint create_renderbuffer(const size_t width, const size_t height);

	void render_frontface();
	void raycasting_pass(const rendercontext& r);
	void star_pass(const rendercontext& r);

	nebulagen::nebula_t m_nebula;

	shader_program m_program_simple;
	shader_program m_program_raycast;

	vao m_va;
	vbo<GLfloat> m_vb;
	boost::optional<state_t> m_state;

	glm::vec3 m_cube_model;
	glm::mat4 m_mvp;

public:
	nebulascene(rendercontext& r);
	nebulascene(const nebulagen::nebula_t& nebula, rendercontext& r);
};
