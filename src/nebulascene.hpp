#pragma once

#include <GL/glew.h>

#include "rendercontext.hpp"

class nebulascene
{
public:
	nebulascene(rendercontext& r)
	{
		r.add_cb(rcphase::init, [&](rendercontext& r) {
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
				glewGetExtension("GL_ARB_shading_language_100") != GL_TRUE)
			{
				std::cerr << "Driver does not support OpenGL Shading Language" << std::endl;
				exit(1);
			}
		});
	}
};
