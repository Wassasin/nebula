#include "nebulascene.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>

#include <glm/gtc/matrix_transform.hpp>

#include "gl.hpp"

static constexpr size_t VOLUME_TEX_SIZE = 128;

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
		std::cerr << "GL_ARB_multitexture support " << std::endl;

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

	size_t texsize = VOLUME_TEX_SIZE;
	size_t size = VOLUME_TEX_SIZE*VOLUME_TEX_SIZE*VOLUME_TEX_SIZE*4;
	GLubyte *data = new GLubyte[size];

	for(size_t i = 0; i < texsize*texsize*texsize; ++i)
	{
		for(size_t j = 0; j < 3; ++j)
			data[i*4+j] = 255;

		data[i*4+3] = 50;
	}

/*
	for(size_t x = 0; x < VOLUME_TEX_SIZE; ++x)
		for(size_t y = 0; y < VOLUME_TEX_SIZE; ++y)
			for(size_t z = 0; z < VOLUME_TEX_SIZE; ++z)
			{
				const glm::ivec3 curvec(x, y, z);

				data[(x*4)   + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = z%250;
				data[(x*4)+1 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = y%250;
				data[(x*4)+2 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 250;
				data[(x*4)+3 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 230;

				{
					glm::ivec3 p = curvec - glm::ivec3(VOLUME_TEX_SIZE-20,VOLUME_TEX_SIZE-30,VOLUME_TEX_SIZE-30);
					if(p.length() < 42)
						data[(x*4)+3 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 0;
				}

				{
					glm::ivec3 p = curvec - glm::ivec3(VOLUME_TEX_SIZE/2,VOLUME_TEX_SIZE/2,VOLUME_TEX_SIZE/2);
					if(p.length() < 24)
						data[(x*4)+3 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 0;
				}

				if(x > 20 && x < 40 && y > 0 && y < VOLUME_TEX_SIZE && z > 10 &&  z < 50)
				{
					data[(x*4)   + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 100;
					data[(x*4)+1 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 250;
					data[(x*4)+2 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = y%100;
					data[(x*4)+3 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 250;
				}

				if(x > 50 && x < 70 && y > 0 && y < VOLUME_TEX_SIZE && z > 10 &&  z < 50)
				{
					data[(x*4)   + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 250;
					data[(x*4)+1 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 250;
					data[(x*4)+2 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = y%100;
					data[(x*4)+3 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 250;
				}

				if(x > 80 && x < 100 && y > 0 && y < VOLUME_TEX_SIZE && z > 10 &&  z < 50)
				{
					data[(x*4)   + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 250;
					data[(x*4)+1 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 70;
					data[(x*4)+2 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = y%100;
					data[(x*4)+3 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 250;
				}

				{
					glm::ivec3 p = curvec - glm::ivec3(24,24,24);
					if(p.length() < 40)
						data[(x*4)+3 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 0;
				}
			}*/

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	gl::generate_textures(1, &volume_texture);
	gl::bind_texture(GL_TEXTURE_3D, volume_texture);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	gl::texture_parameter_i(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl::texture_parameter_i(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl::texture_parameter_i(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	gl::texture_parameter_i(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	gl::texture_parameter_i(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	gl::texture_image_3d(GL_TEXTURE_3D, 0, GL_RGBA, VOLUME_TEX_SIZE, VOLUME_TEX_SIZE,VOLUME_TEX_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

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

void nebulascene::render_backface(const glm::mat4& mvp)
{
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_state->backface_texture, 0);
	gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_program_simple.use();
	m_program_simple.uniform<glm::mat4>("mvp").set(mvp);

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

void nebulascene::raycasting_pass(const glm::mat4& mvp)
{
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_state->final_texture, 0);
	gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_program_raycast.use();
	m_program_raycast.uniform<glm::mat4>("mvp").set(mvp);
	m_program_raycast.uniform<GLfloat>("stepsize").set(1.0f/50.0f);

	m_program_raycast.bind_attribute(m_state->backface_texture, "tex");
	m_program_raycast.bind_attribute(m_state->volume_texture, "volume_tex");

	gl::enable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	gl::enable_vertex_attribute_array(0);
	m_vb.bind(GL_ARRAY_BUFFER);
	gl::vertex_attribute_pointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	gl::draw_arrays(GL_QUADS, 0, 4*6);
	glDisableVertexAttribArray(0);

	gl::disable(GL_CULL_FACE);

	gl::use_program(0);
}

void nebulascene::render_buffer_to_screen(const size_t width, const size_t height)
{
	gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);

	gl::bind_texture(GL_TEXTURE_2D, m_state->final_texture);
	//gl::bind_texture(GL_TEXTURE_2D, m_state->backface_texture);

	/* Reshape for projection */
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);
	glMatrixMode(GL_MODELVIEW);

	/* Draw the texture */
	glDisable(GL_DEPTH_TEST);
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(0.0f, 0.0f);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(1.0f, 0.0f);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(1.0f, 1.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(0.0f, 1.0f);

	glEnd();
	glEnable(GL_DEPTH_TEST);

	glDisable(GL_TEXTURE_2D);
}

nebulascene::nebulascene(rendercontext &r)
: m_program_simple(false)
, m_program_raycast(false)
, m_va(false)
, m_vb(false)
, m_state()
{
	r.add_cb(rcphase::init, [&](rendercontext& r) {
		check_support();

		gl::clear_color(0.1f, 0.1f, 0.1f, 0.0f);

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

		GLuint backface_texture = create_2dtexture(r.size().first, r.size().second);
		GLuint final_texture = create_2dtexture(r.size().first, r.size().second);

		GLuint renderbuffer = create_renderbuffer(r.size().first, r.size().second);

		m_state.reset({
			0,
			volume_texture,
			framebuffer,
			backface_texture,
			final_texture,
			renderbuffer
		});
	});

	r.add_cb(rcphase::update, [&](rendercontext& r) {
		glm::mat4 projection = glm::perspective(60.0f, (GLfloat)r.size().first/(GLfloat)r.size().second, 1.0f, 100.0f);

		glm::mat4 anim = glm::rotate(
			glm::mat4(),
			(GLfloat)m_state->rotate_i,
			glm::vec3(1.0f, 0.0f, 1.0f)
		);

		// Model needs to be centered
		glm::mat4 model = glm::translate(
			glm::mat4(),
			glm::vec3(-0.5f, -0.5f, -0.5f)
		);

		glm::mat4 view = glm::lookAt(
			glm::vec3(0.0f, 0.0f, 2.25f),
			glm::vec3(0.0f, 0.0f, 2.0f),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);

		r.mvp = projection * view * anim * model;

		m_state->rotate_i = (m_state->rotate_i + 1) % 360;
	});

	r.add_cb(rcphase::draw, [&](rendercontext& r) {
		/* Enable renderbuffers */
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_state->framebuffer);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_state->renderbuffer);

		render_backface(r.mvp);
		raycasting_pass(r.mvp);

		/* Disable renderbuffers */
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		render_buffer_to_screen(r.size().first, r.size().second);
	});
}
