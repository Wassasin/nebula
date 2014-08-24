#pragma once

#include <boost/optional.hpp>
#include <memory>

#include "gl/rendercontext.hpp"
#include "gl/shader.hpp"
#include "gl/vao.hpp"
#include "gl/vbo.hpp"

#include "nebula.hpp"

class nebulaparticlescene
{
private:
	struct state_t
	{
		GLuint billboard_vertex_buffer;
		GLuint particles_position_buffer;
		GLuint particles_color_buffer;
		GLuint particle_texture;
	};

	static void check_support();
	void update_particles(const rendercontext& r);

	void particle_pass(const rendercontext& r);
	void star_pass(const rendercontext& r);

	particle_nebula_t m_nebula;

	shader_program m_program_particle;

	vao m_va;
	vbo<GLfloat> m_vb;
	boost::optional<state_t> m_state;

	glm::vec3 m_cube_model;
	glm::mat4 m_mvp;

	std::unique_ptr<GLfloat[]> m_particule_position_size_data;
	std::unique_ptr<GLubyte[]> m_particule_color_data;

public:
	nebulaparticlescene(const particle_nebula_t& nebula, rendercontext& r);
};
