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
	struct rawparticle_t
	{
		glm::vec3 pos;
		GLfloat size;
		glm::vec4 color;

		GLfloat tmpz;
	};

	struct layer_t
	{
		std::vector<rawparticle_t> particles;
		GLuint particle_buffer;

		layer_t(const size_t size);
	};

	enum class particle_type : uint8_t
	{
		PT_BLEED,
		PT_DUST,
		PT_COUNT
	};

	struct state_t
	{
		GLuint billboard_vertex_buffer;
		GLuint particle_texture, star_texture;

		std::array<layer_t, (size_t)particle_type::PT_COUNT> layers;
	};

	static void check_support();
	void update_particles(layer_t& layer, const rendercontext& r);
	void draw_particles(const layer_t& layer, GLuint texture, const rendercontext& r);

	particle_nebula_t m_nebula;

	shader_program m_program_particle;

	vao m_va;
	vbo<GLfloat> m_vb;
	boost::optional<state_t> m_state;

	glm::vec3 m_cube_model;
	glm::mat4 m_mvp;

public:
	nebulaparticlescene(const particle_nebula_t& nebula, rendercontext& r);
};
