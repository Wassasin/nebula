#pragma once

#include <GL/glew.h>
#include <boost/optional.hpp>

#include "rendercontext.hpp"
#include "shader.hpp"
#include "vao.hpp"
#include "vbo.hpp"

class nebulascene
{
private:
	struct state_t
	{
		size_t rotate_i;

		GLuint volume_texture;

		GLuint framebuffer;

		GLuint backface_texture;
		GLuint final_texture;

		GLuint renderbuffer;
	};

	static void check_support();
	static GLuint create_volumetexture();
	static GLuint create_2dtexture(const size_t width, const size_t height);
	static GLuint create_renderbuffer(const size_t width, const size_t height);

	void render_backface(const glm::mat4& mvp);
	void raycasting_pass(const glm::mat4& mvp);
	void render_buffer_to_screen(const size_t width, const size_t height);

	shader_program m_program_simple;
	shader_program m_program_raycast;

	vao m_va;
	vbo<GLfloat> m_vb;
	boost::optional<state_t> m_state;

public:
	nebulascene(rendercontext& r);
};
