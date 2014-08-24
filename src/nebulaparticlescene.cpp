#include "nebulaparticlescene.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gl/gl.hpp"
#include "gl/texture.hpp"

#include "nebulagen.hpp"
#include "particlelighting.hpp"

void nebulaparticlescene::check_support()
{
	glewGetExtension("glMultiTexCoord2fvARB");
	if(glewGetExtension("GL_EXT_framebuffer_object") )
		std::cerr << "GL_EXT_framebuffer_object support " << std::endl;

	if(glewGetExtension("GL_EXT_renderbuffer_object"))
		std::cerr << "GL_EXT_renderbuffer_object support " << std::endl;

	if(glewGetExtension("GL_ARB_vertex_buffer_object"))
		std::cerr << "GL_ARB_vertex_buffer_object support" << std::endl;

	if(GL_ARB_multitexture)
		std::cerr << "GL_ARB_multitexture support" << std::endl;

	if(glewGetExtension("GL_ARB_fragment_shader"))
		std::cerr << "GL_ARB_fragment_shader support" << std::endl;

	if (glewGetExtension("GL_ARB_fragment_shader")      != GL_TRUE ||
		glewGetExtension("GL_ARB_vertex_shader")        != GL_TRUE ||
		glewGetExtension("GL_ARB_shader_objects")       != GL_TRUE ||
		glewGetExtension("GL_ARB_shading_language_100") != GL_TRUE
	)
		throw std::runtime_error("Driver does not support OpenGL Shading Language");
}

void nebulaparticlescene::update_particles(const rendercontext& r)
{
	const glm::mat4 cube_modelmat = glm::translate(glm::mat4(), m_cube_model);
	const glm::mat4 mvp = m_mvp * cube_modelmat;

	for(particle_t& p : m_nebula.particles)
		p.z = (mvp * glm::vec4(p.pos.x, p.pos.y, p.pos.z, 0.0f)).z;

	std::sort(m_nebula.particles.begin(), m_nebula.particles.end(), [&](const particle_t& a, const particle_t& b)
	{
		return a.z > b.z;
	});
}

void nebulaparticlescene::particle_pass(const rendercontext& r)
{
	const glm::mat4 cube_modelmat = glm::translate(glm::mat4(), m_cube_model);

	{
		size_t i = 0;

		for(const particle_t& p : m_nebula.particles)
		{
			m_particule_position_size_data[4*i+0] = p.pos.x;
			m_particule_position_size_data[4*i+1] = p.pos.y;
			m_particule_position_size_data[4*i+2] = p.pos.z;
			m_particule_position_size_data[4*i+3] = (p.color.a/255.0f) * 0.004f;

			m_particule_color_data[4*i+0] = p.color.r;
			m_particule_color_data[4*i+1] = p.color.g;
			m_particule_color_data[4*i+2] = p.color.b;
			m_particule_color_data[4*i+3] = p.color.a*0.08f;

			i++;
		}
	}

	gl::bind_buffer(GL_ARRAY_BUFFER, m_state->particles_position_buffer);
	glBufferData(GL_ARRAY_BUFFER, m_nebula.particles.size() * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_nebula.particles.size() * sizeof(GLfloat) * 4, m_particule_position_size_data.get());

	gl::bind_buffer(GL_ARRAY_BUFFER, m_state->particles_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, m_nebula.particles.size() * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_nebula.particles.size() * sizeof(GLubyte) * 4, m_particule_color_data.get());

	gl::enable(GL_BLEND);
	gl::blend_function(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_program_particle.use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_state->particle_texture);

	m_program_particle.uniform<GLint>("textureSampler").set(0);

	glm::mat4 vp = r.camera.to_matrix();

	m_program_particle.uniform<glm::vec3>("CameraRight_worldspace").set(glm::vec3(vp[0][0], vp[1][0], vp[2][0]));
	m_program_particle.uniform<glm::vec3>("CameraUp_worldspace").set(glm::vec3(vp[0][1], vp[1][1], vp[2][1]));
	m_program_particle.uniform<glm::mat4>("VP").set(m_mvp * cube_modelmat);

	gl::enable_vertex_attribute_array(0);
	gl::bind_buffer(GL_ARRAY_BUFFER, m_state->billboard_vertex_buffer);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE, // normalized?
		0, // stride
		(void*)0 // array buffer offset
	);

	gl::enable_vertex_attribute_array(1);
	gl::bind_buffer(GL_ARRAY_BUFFER, m_state->particles_position_buffer);
	glVertexAttribPointer(
		1,
		4,
		GL_FLOAT,
		GL_FALSE, // normalized?
		0, // stride
		(void*)0 // array buffer offset
	);

	gl::enable_vertex_attribute_array(2);
	gl::bind_buffer(GL_ARRAY_BUFFER, m_state->particles_color_buffer);
	glVertexAttribPointer(
		2,
		4,
		GL_UNSIGNED_BYTE,
		GL_TRUE, // normalized? *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
		0, // stride
		(void*)0 // array buffer offset
	);

	glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
	glVertexAttribDivisor(1, 1); // positions : one per quad (its center) -> 1
	glVertexAttribDivisor(2, 1); // color : one per quad -> 1

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, m_nebula.particles.size());

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	gl::disable(GL_BLEND);

	gl::use_program(0);
}

void nebulaparticlescene::star_pass(const rendercontext& r)
{
	const glm::mat4 cube_modelmat = glm::translate(glm::mat4(), m_cube_model);

	glPointSize(5.0);
	glBegin(GL_POINTS);

	for(const auto& star : m_nebula.stars)
	{
		glm::vec4 starpos(star.pos.x, star.pos.y, star.pos.z, 1.0);
		starpos = (m_mvp * cube_modelmat) * starpos;

		glColor3f(star.color.r/255.0, star.color.g/255.0, star.color.b/255.0);
		glVertex4f(starpos.x, starpos.y, starpos.z, starpos.w);
	}

	glEnd();
}

nebulaparticlescene::nebulaparticlescene(const particle_nebula_t& nebula, rendercontext& r)
: m_nebula(nebula)
, m_program_particle(false)
, m_state()
, m_cube_model(-0.5f, -0.5f, -0.5f)
, m_mvp()
{
	r.add_cb(rcphase::init, [&](rendercontext& r) {
		check_support();

		gl::clear_color(0.0f, 0.0f, 0.0f, 0.0f);

		m_program_particle.attach(shader::from_file(shader_type::vertex, "shaders/nebulaparticle.vertexshader"));
		m_program_particle.attach(shader::from_file(shader_type::fragment, "shaders/nebulaparticle.fragmentshader"));
		m_program_particle.link();

		static const GLfloat g_vertex_buffer_data[] = {
			-0.5f, -0.5f, 0.0f,
			0.5f, -0.5f, 0.0f,
			-0.5f, 0.5f, 0.0f,
			0.5f, 0.5f, 0.0f,
		};

		GLuint billboard_vertex_buffer;
		glGenBuffers(1, &billboard_vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

		GLuint particles_position_buffer;
		glGenBuffers(1, &particles_position_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);

		// Initialize with empty (NULL) buffer : it will be updated later, each frame.
		glBufferData(GL_ARRAY_BUFFER, m_nebula.particles.size() * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

		// The VBO containing the colors of the particles
		GLuint particles_color_buffer;
		glGenBuffers(1, &particles_color_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);

		// Initialize with empty (NULL) buffer : it will be updated later, each frame.
		glBufferData(GL_ARRAY_BUFFER, m_nebula.particles.size() * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

		m_state.reset({
			billboard_vertex_buffer,
			particles_position_buffer,
			particles_color_buffer,
			texture::loadDDS("textures/particle.DDS")
		});

		m_particule_position_size_data.reset(new GLfloat[m_nebula.particles.size() * 4]);
		m_particule_color_data.reset(new GLubyte[m_nebula.particles.size() * 4]);
	});

	r.add_cb(rcphase::init, [&](rendercontext& r) {
		r.camera.position = glm::vec3(-1.0f, 0.0f, -1.0f);
		r.camera.rotation = glm::vec2(0.0f, 0.0f);
	});

	r.add_cb(rcphase::update, [&](rendercontext& r) {
		glm::mat4 projection = glm::perspective(60.0f, (GLfloat)r.size().first/(GLfloat)r.size().second, 0.1f, 100.0f);
		m_mvp = projection * r.camera.to_matrix();

		update_particles(r);
	});

	r.add_cb(rcphase::draw, [&](rendercontext& r) {
		gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		particle_pass(r);
		star_pass(r);
	});
}
