#pragma once

#include <iostream>
#include <boost/program_options.hpp>

#include "gl/glfwcontext.hpp"
#include "gl/glutcontext.hpp"

#include "util/cache.hpp"

#include "nebulagen.hpp"
#include "volumelighting.hpp"
#include "volumeparticletransform.hpp"
#include "particlelighting.hpp"

#include "simplescene.hpp"
#include "nebulascene.hpp"
#include "nebulaparticlescene.hpp"

class cli
{
	cli() = delete;
	cli(cli&) = delete;
	cli& operator=(cli&) = delete;

	enum class context
	{
		CONTEXT_GLUT,
		CONTEXT_GLFW
	};

	enum class scene
	{
		SCENE_VOLUME,
		SCENE_PARTICLE
	};

	struct options
	{
		context c;
		scene s;

		int seed = 4821903;
	};

	static int interpret(options& opt, int argc, char** argv)
	{
		std::string context_str, scene_str;

		boost::program_options::options_description o_general("General options");
		o_general.add_options()
				("help,h", "display this message")
				("context,c", boost::program_options::value(&context_str), "{glut, glfw} GL context library (defaults to glfw)")
				("scene,s", boost::program_options::value(&scene_str), "{volume, particle} render method (defaults to particle)")
				("seed,i", boost::program_options::value(&opt.seed), "any number (defaults to 4821903)");

		boost::program_options::variables_map vm;
		boost::program_options::positional_options_description pos;

		boost::program_options::options_description options("Allowed options");
		options.add(o_general);

		try
		{
			boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(options).positional(pos).run(), vm);
		} catch(boost::program_options::unknown_option &e)
		{
			std::cerr << "Unknown option --" << e.get_option_name() << ", see --help" << std::endl;
			return 2;
		}

		try
		{
			boost::program_options::notify(vm);
		} catch(const boost::program_options::required_option &e)
		{
			std::cerr << "You forgot this: " << e.what() << std::endl;
			return 1;
		}

		if(vm.count("help"))
		{
			std::cout
					<< "Volumetric Particle Clouds and Raycasting in OpenGL and C++11. [https://github.com/Wassasin/nebula]" << std::endl
					<< "Usage: ./nebula [options]" << std::endl
					<< std::endl
					<< o_general;

			return 1;
		}

		if(context_str == "glfw" || context_str == "")
			opt.c = context::CONTEXT_GLFW;
		else if(context_str == "glut")
			opt.c = context::CONTEXT_GLUT;
		else
		{
			std::cerr << "Unrecognized context \"" << context_str << "\"" << std::endl;
			return 1;
		}

		if(scene_str == "particle" || scene_str == "")
			opt.s = scene::SCENE_PARTICLE;
		else if(scene_str == "volume")
			opt.s = scene::SCENE_VOLUME;
		else
		{
			std::cerr << "Unrecognized scene \"" << scene_str << "\"" << std::endl;
			return 1;
		}

		return 0;
	}

	static nebulagen::nebula_t acquire_volume(const options& opt)
	{
		return cache<nebulagen::nebula_t>::acquire("volume.msgpack.gz", [&](){
			nebulagen gen(opt.seed);
			return gen.generate();
		});
	}

	static nebulagen::nebula_t acquire_volume_lighted(const options& opt)
	{
		return cache<nebulagen::nebula_t>::acquire("volume_lighted.msgpack.gz", [&](){
			nebulagen::nebula_t nebula = acquire_volume(opt);
			volumelighting<nebulagen::X, nebulagen::Y, nebulagen::Z>::apply_lighting(nebula);
			return nebula;
		});
	}

	static particle_nebula_t acquire_particles(const options& opt)
	{
		return cache<particle_nebula_t>::acquire("particles.msgpack.gz", [&](){
			particle_nebula_t pnebula;

			{
				nebulagen::nebula_t nebula = acquire_volume(opt);
				pnebula = particle_nebula_t(volume_to_particles(nebula.dust, opt.seed), nebula.stars);
			}

			particlelighting::apply_lighting(pnebula);
			return pnebula;
		});
	}

	template<typename RENDERER>
	static int render(const options& opt, int argc, char** argv)
	{
		switch(opt.s)
		{
		case scene::SCENE_VOLUME:
		{
			auto volume_lighted = acquire_volume_lighted(opt);

			RENDERER r;
			nebulascene s(volume_lighted, r);
			r.run(argc, argv);
			return 0;
		}
		case scene::SCENE_PARTICLE:
		{
			auto particles = acquire_particles(opt);

			RENDERER r;
			r.add_cb(rcphase::draw, [&](rendercontext& r) {
				gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			});

			/*r.add_cb(rcphase::draw, [&](rendercontext& r) {
				glm::vec3 model = particles.stars[0].pos + glm::vec3(-0.5f, -0.5f, -0.5f);

				glm::mat4 projection = glm::perspective(60.0f, (GLfloat)r.size().first/(GLfloat)r.size().second, 0.1f, 100.0f);
				glm::mat4 modelmat = glm::translate(glm::mat4(), model);
				glm::mat4 mat = projection * r.camera.to_matrix() * modelmat;

				glPushMatrix();
				glLoadMatrixf((GLfloat*)&mat);

				particlelighting::draw_debug();

				glBegin(GL_LINES);
				glColor3f(1.0f, 1.0f, 1.0f);
				glm::vec3 orig;
				glm::vec3 dir(-0.979421, 0.19943, -0.0310227);
				glm::vec3 dest = orig + dir;

				glVertex3f(orig.x, orig.y, orig.z);
				glVertex3f(dest.x, dest.y, dest.z);
				glEnd();

				glPopMatrix();
			});*/

			nebulaparticlescene s(particles, r);
			r.run(argc, argv);
			return 0;
		}
		default:
			std::cerr << "Unknown scene (logic error)" << std::endl;
			return 1;
		}
	}

	static int act(const options& opt, int argc, char** argv)
	{
		switch(opt.c)
		{
		case context::CONTEXT_GLFW:
			return render<glfwcontext>(opt, argc, argv);
		case context::CONTEXT_GLUT:
			return render<glutcontext>(opt, argc, argv);
		default:
			std::cerr << "Unknown context (logic error)" << std::endl;
			return 1;
		}
	}

public:
	static int run(int argc, char** argv)
	{
		options opt;
		int result = interpret(opt, argc, argv);
		if(result != 0)
			return result;
		return act(opt, argc, argv);
	}
};
