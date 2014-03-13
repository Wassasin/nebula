#include "nebulascene.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gl.hpp"
#include "perlin.hpp"
#include "nebulagen.hpp"

static constexpr size_t SIZE = 128;

void nebulascene::check_support()
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

GLuint nebulascene::create_volumetexture()
{
	GLuint volume_texture;

	static constexpr size_t size = SIZE*SIZE*SIZE*4;

	nebulagen<SIZE> generator(8794891);
	GLubyte *data = new GLubyte[size];

	auto volume = generator.generate();
	std::cerr << sizeof(volume) << std::endl;

	for(size_t x = 0; x < SIZE; ++x)
		for(size_t y = 0; y < SIZE; ++y)
			for(size_t z = 0; z < SIZE; ++z)
			{
				size_t i = x * SIZE * SIZE + y * SIZE + z;
				glm::uvec3 pos(x, y, z);
				glm::vec4 v = volume[pos];

				data[i*4+0] = 255 * v.r;
				data[i*4+1] = 255 * v.g;
				data[i*4+2] = 255 * v.b;
				data[i*4+3] = 255 * v.a;
			}

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	gl::generate_textures(1, &volume_texture);
	gl::bind_texture(GL_TEXTURE_3D, volume_texture);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	gl::texture_parameter_i(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::texture_parameter_i(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl::texture_parameter_i(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	gl::texture_parameter_i(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	gl::texture_parameter_i(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	gl::texture_image_3d(GL_TEXTURE_3D, 0, GL_RGBA, SIZE, SIZE, SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	delete []data;
	std::cerr << "Volume texture created" << std::endl;

	return volume_texture;
}

GLuint nebulascene::create_2dtexture(const size_t width, const size_t height)
{
	GLuint texture;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA16F_ARB, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

	return texture;
}

GLuint nebulascene::create_renderbuffer(const size_t width, const size_t height)
{
	GLuint renderbuffer;

	glGenRenderbuffersEXT(1, &renderbuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderbuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, renderbuffer);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	return renderbuffer;
}

void nebulascene::render_frontface()
{
	/* Enable renderbuffers */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_state->framebuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_state->renderbuffer);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_state->frontface_texture, 0);
	gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const glm::mat4 cube_modelmat = glm::translate(glm::mat4(), m_cube_model);

	m_program_simple.use();
	m_program_simple.uniform<glm::mat4>("mvp").set(m_mvp * cube_modelmat);

	gl::enable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	gl::enable_vertex_attribute_array(0);
	m_vb.bind(GL_ARRAY_BUFFER);
	gl::vertex_attribute_pointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	gl::draw_arrays(GL_QUADS, 0, 4*6);
	glDisableVertexAttribArray(0);

	gl::disable(GL_CULL_FACE);

	gl::use_program(0);

	/* Disable renderbuffers */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void nebulascene::raycasting_pass(const rendercontext& r)
{
	constexpr GLfloat margin = 0.2f;

	bool inside_volume = (
		r.camera.position.x >= -0.5f - margin && r.camera.position.x <= 0.5f + margin &&
		r.camera.position.y >= -0.5f - margin && r.camera.position.y <= 0.5f + margin &&
		r.camera.position.z >= -0.5f - margin && r.camera.position.z <= 0.5f + margin
	);

	if(!inside_volume)
		render_frontface();

	const glm::mat4 cube_modelmat = glm::translate(glm::mat4(), m_cube_model);

	//glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_state->final_texture, 0);
	gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_program_raycast.use();

	gl::active_texture(GL_TEXTURE0 + 0);
	gl::bind_texture(GL_TEXTURE_2D, m_state->frontface_texture);
	m_program_raycast.uniform<GLint>("frontface_tex").set(0);

	gl::active_texture(GL_TEXTURE0 + 1);
	gl::bind_texture(GL_TEXTURE_3D, m_state->volume_texture);
	m_program_raycast.uniform<GLint>("volume_tex").set(1);

	m_program_raycast.uniform<GLint>("inside_volume").set(inside_volume);
	m_program_raycast.uniform<glm::mat4>("mvp").set(m_mvp * cube_modelmat);
	m_program_raycast.uniform<glm::vec3>("camerapos").set(r.camera.position - m_cube_model);
	m_program_raycast.uniform<GLfloat>("stepsize").set(0.5f/((GLfloat)SIZE));

	gl::enable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	gl::enable_vertex_attribute_array(0);
	m_vb.bind(GL_ARRAY_BUFFER);
	gl::vertex_attribute_pointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	gl::draw_arrays(GL_QUADS, 0, 4*6);
	glDisableVertexAttribArray(0);

	gl::disable(GL_CULL_FACE);

	gl::use_program(0);
}

nebulascene::nebulascene(rendercontext &r)
: m_program_simple(false)
, m_program_raycast(false)
, m_va(false)
, m_vb(false)
, m_state()
, m_cube_model(-0.5f, -0.5f, -0.5f)
, m_mvp()
{
	r.add_cb(rcphase::init, [&](rendercontext& r) {
		check_support();

		gl::clear_color(0.0f, 0.0f, 0.0f, 0.0f);

		m_program_simple.attach(shader::from_file(shader_type::vertex, "shaders/nebulasimple.vertexshader"));
		m_program_simple.attach(shader::from_file(shader_type::fragment, "shaders/nebulasimple.fragmentshader"));
		m_program_simple.link();

		m_program_raycast.attach(shader::from_file(shader_type::vertex, "shaders/nebularaycast.vertexshader"));
		m_program_raycast.attach(shader::from_file(shader_type::fragment, "shaders/nebularaycast.fragmentshader"));
		m_program_raycast.link();

		m_va.bind();

		m_vb.bind(GL_ARRAY_BUFFER);
		m_vb.data({
			0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
			1.0f, 0.0f, 0.0f,

			// Front side
			0.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			0.0f, 1.0f, 1.0f,

			// Top side
			0.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 0.0f,

			// Bottom side
			0.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f,

			// Left side
			0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f,
			0.0f, 1.0f, 1.0f,
			0.0f, 1.0f, 0.0f,

			// Right side
			1.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 0.0f, 1.0f,
		}, GL_STATIC_DRAW);

		GLuint volume_texture = create_volumetexture();

		/* Create framebuffer */
		GLuint framebuffer;
		glGenFramebuffersEXT(1, &framebuffer);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);

		GLuint frontface_texture = create_2dtexture(r.size().first, r.size().second);
		GLuint final_texture = create_2dtexture(r.size().first, r.size().second);

		GLuint renderbuffer = create_renderbuffer(r.size().first, r.size().second);

		m_state.reset({
			volume_texture,
			framebuffer,
			frontface_texture,
			final_texture,
			renderbuffer
		});
	});

	r.add_cb(rcphase::init, [&](rendercontext& r) {
		r.camera.position = glm::vec3(0.0f, 0.0f, -2.0f);
		r.camera.rotation = glm::vec2(0.0f, 0.0f);
	});

	r.add_cb(rcphase::update, [&](rendercontext& r) {
		glm::mat4 projection = glm::perspective(60.0f, (GLfloat)r.size().first/(GLfloat)r.size().second, 0.1f, 100.0f);
		m_mvp = projection * r.camera.to_matrix();
	});

	r.add_cb(rcphase::draw, [&](rendercontext& r) {
		raycasting_pass(r);

		//render_buffer_to_screen(r.size().first, r.size().second);
	});
}
