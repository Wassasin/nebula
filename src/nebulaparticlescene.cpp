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

void nebulaparticlescene::update_particles(layer_t& layer, const rendercontext& r)
{
	const glm::mat4 cube_modelmat = glm::translate(glm::mat4(), m_cube_model);
	const glm::mat4 mvp = m_mvp * cube_modelmat;

	const size_t size = layer.particles.size();

	for(size_t i = 0; i < size; ++i)
	{
		glm::vec3 pos(layer.particles[i].pos);
		layer.zs[i] = (mvp * glm::vec4(pos.x, pos.y, pos.z, 0.0f)).z;
	}

	std::sort(layer.indices.begin(), layer.indices.end(), [&](const size_t i, const size_t j)
	{
		return layer.zs[i] > layer.zs[j];
	});
}

void nebulaparticlescene::draw_particles(const layer_t& layer, GLuint texture, const rendercontext& r)
{
	const glm::mat4 cube_modelmat = glm::translate(glm::mat4(), m_cube_model);

	gl::bind_buffer(GL_ARRAY_BUFFER, layer.particle_vertexsize_buffer);
	glBufferData(GL_ARRAY_BUFFER, layer.particles.size() * sizeof(rawparticle_t), NULL, GL_STREAM_DRAW); // Buffer orphaning
	glBufferSubData(GL_ARRAY_BUFFER, 0, layer.particles.size() * sizeof(rawparticle_t), layer.particles.data());

	gl::bind_buffer(GL_ARRAY_BUFFER, layer.particle_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, layer.particles.size() * sizeof(rawparticle_t), NULL, GL_STREAM_DRAW); // Buffer orphaning
	glBufferSubData(GL_ARRAY_BUFFER, 0, layer.particles.size() * sizeof(rawparticle_t), layer.particles.data());

	gl::enable(GL_BLEND);
	gl::blend_function(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_program_particle.use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

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

	// Vertices + size
	gl::enable_vertex_attribute_array(1);
	gl::bind_buffer(GL_ARRAY_BUFFER, layer.particle_vertexsize_buffer);
	glVertexAttribPointer(
		1,
		4,
		GL_FLOAT,
		GL_FALSE, // normalized?
		sizeof(rawparticle_t), // stride
		(void*)0 // array buffer offset
	);

	gl::enable_vertex_attribute_array(2);
	gl::bind_buffer(GL_ARRAY_BUFFER, layer.particle_color_buffer);
	glVertexAttribPointer(
		2,
		4,
		GL_FLOAT,
		GL_FALSE, // normalized?
		sizeof(rawparticle_t), // stride
		(void*) (sizeof(decltype(rawparticle_t::pos)) + sizeof(decltype(rawparticle_t::size))) // array buffer offset
	);

	glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
	glVertexAttribDivisor(1, 1); // positions : one per quad (its center) -> 1
	glVertexAttribDivisor(2, 1); // color : one per quad -> 1

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, layer.particles.size());

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	gl::disable(GL_BLEND);

	gl::use_program(0);
}

GLuint create_particle_texture()
{
	static constexpr size_t size = 1024;
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	GLubyte data[size][size][4];
	for(size_t x = 0; x < size; ++x)
		for(size_t y = 0; y < size; ++y)
		{
			GLfloat dist = glm::clamp(glm::distance(glm::vec2(0.5f, 0.5f), glm::vec2(x, y)/(GLfloat)size)*2.0f, 0.0f, 1.0f);
			data[x][y][0] = 255;
			data[x][y][1] = 255;
			data[x][y][2] = 255;
			data[x][y][3] = glm::clamp(255.0f * 64.0f * (1.0f - std::pow(dist, 0.01f)), 0.0f, 255.0f);
		}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	return textureID;
}

GLuint create_star_texture()
{
	static constexpr size_t size = 1024;
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	GLubyte data[size][size][4];
	for(size_t x = 0; x < size; ++x)
		for(size_t y = 0; y < size; ++y)
		{
			GLfloat dist = glm::clamp(glm::distance(glm::vec2(0.5f, 0.5f), glm::vec2(x, y)/(GLfloat)size)*2.0f, 0.0f, 1.0f);
			data[x][y][0] = 255;
			data[x][y][1] = 255;
			data[x][y][2] = 255;
			data[x][y][3] = glm::clamp(255.0f * 64.0f * (1.0f - std::pow(dist, 0.01f)), 0.0f, 255.0f);
		}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	return textureID;
}

nebulaparticlescene::layer_t::layer_t(const size_t size)
: particles()
, zs(size)
, indices(size)
, particle_vertexsize_buffer()
, particle_color_buffer()
{
	particles.reserve(size);
	std::iota(indices.begin(), indices.end(), 0);

	glGenBuffers(1, &particle_vertexsize_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particle_vertexsize_buffer);
	glBufferData(GL_ARRAY_BUFFER, size * sizeof(rawparticle_t), NULL, GL_STREAM_DRAW); // Initialize with empty (NULL) buffer : it will be updated later, each frame.

	glGenBuffers(1, &particle_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particle_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, size * sizeof(rawparticle_t), NULL, GL_STREAM_DRAW); // Initialize with empty (NULL) buffer : it will be updated later, each frame.
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
	});

	r.add_cb(rcphase::init, [&](rendercontext& r) {
		m_program_particle.attach(shader::from_file(shader_type::vertex, "shaders/nebulaparticle.vertexshader"));
		m_program_particle.attach(shader::from_file(shader_type::fragment, "shaders/nebulaparticle.fragmentshader"));
		m_program_particle.link();
	});

	r.add_cb(rcphase::init, [&](rendercontext& r) {
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

		layer_t layer_bleed(0);
		layer_t layer_dust(m_nebula.particles.size() + m_nebula.stars.size());
		for(const particle_t& p : m_nebula.particles)
			layer_dust.particles.emplace_back(rawparticle_t({
				glm::vec3(p.pos.x, p.pos.y, p.pos.z),
				(p.color.a/255.0f) * 0.004f,
				glm::vec4(p.color.r/255.0f, p.color.g/255.0f, p.color.b/255.0f, p.color.a / 255.0f * 0.5f)
			}));

		for(const star_t& s : m_nebula.stars)
			layer_dust.particles.emplace_back(rawparticle_t({
				glm::vec3(s.pos.x, s.pos.y, s.pos.z),
				0.1f,
				glm::vec4(s.color.r/255.0f, s.color.g/255.0f, s.color.b/255.0f, 1.0f)
			}));

		m_state.reset({
			billboard_vertex_buffer,
			create_particle_texture(),
			create_star_texture(),
			{{
				layer_bleed,
				layer_dust
			}}
		});
	});

	r.add_cb(rcphase::init, [&](rendercontext& r) {
		r.camera.position = glm::vec3(-1.0f, 0.0f, -1.0f);
		r.camera.rotation = glm::vec2(0.0f, 0.0f);
	});

	r.add_cb(rcphase::update, [&](rendercontext& r) {
		glm::mat4 projection = glm::perspective(60.0f, (GLfloat)r.size().first/(GLfloat)r.size().second, 0.01f, 100.0f);
		m_mvp = projection * r.camera.to_matrix();
	});

	r.add_cb(rcphase::update, [&](rendercontext& r) {
		for(layer_t& l : m_state->layers)
			update_particles(l, r);
	});

	r.add_cb(rcphase::draw, [&](rendercontext& r) {
		draw_particles(m_state->layers[(size_t)particle_type::PT_DUST], m_state->particle_texture, r);
	});
}
