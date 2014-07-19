#pragma once

#include <GL/glew.h>
#include <boost/optional.hpp>
#include <memory>

#include "rendercontext.hpp"
#include "shader.hpp"
#include "vao.hpp"
#include "vbo.hpp"

#include "nebulagen.hpp"

class nebulaparticlescene
{
private:
	struct particle_t
	{
		glm::vec3 pos;
		glm::uvec4 color;
		GLfloat z;
	};

	struct state_t
	{
		GLuint billboard_vertex_buffer;
		GLuint particles_position_buffer;
		GLuint particles_color_buffer;
		GLuint particle_texture;
	};

	static void check_support();
	void init_particles();
	void update_particles(const rendercontext& r);

	void particle_pass(const rendercontext& r);
	void star_pass(const rendercontext& r);

	int m_seed;
	nebulagen::nebula_t m_nebula;
	std::vector<particle_t> m_particles;

	shader_program m_program_particle;

	vao m_va;
	vbo<GLfloat> m_vb;
	boost::optional<state_t> m_state;

	glm::vec3 m_cube_model;
	glm::mat4 m_mvp;

	std::unique_ptr<GLfloat[]> m_particule_position_size_data;
	std::unique_ptr<GLubyte[]> m_particule_color_data;

public:
	nebulaparticlescene(rendercontext& r);
	nebulaparticlescene(const nebulagen::nebula_t& nebula, rendercontext& r);
};
