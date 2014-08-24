#include <boost/filesystem.hpp>

#include "gl/glfwcontext.hpp"
#include "gl/glutcontext.hpp"

#include "nebulagen.hpp"
#include "volumelighting.hpp"

#include "simplescene.hpp"
#include "nebulascene.hpp"
#include "nebulaparticlescene.hpp"

#include "util/msgpackreader.hpp"
#include "util/msgpackwriter.hpp"

void add_debug_lines(rendercontext& r)
{
	r.add_cb(rcphase::draw, [](rendercontext& s) {
		glLineWidth(2.0);
		glBegin(GL_LINES);
		for(qi i = 0; i < 3; i++)
		{
			glColor3f(i == 0, i == 1, i == 2);
			glVertex3f(0.0, 0.0, 0.0);
			glVertex3f(i == 0, i == 1, i == 2);
		}
		glEnd();
	});
}

nebulagen::nebula_t init_nebula(const std::string filename, const size_t seed)
{
	if(boost::filesystem::exists(filename))
	{
		MsgpackReader<nebulagen::nebula_t> reader(filename);

		nebulagen::nebula_t result;
		reader.read(result);
		return result;
	}
	else
	{
		nebulagen gen(seed);
		MsgpackWriter<nebulagen::nebula_t> writer(filename);

		nebulagen::nebula_t result = gen.generate();
		volumelighting<nebulagen::X, nebulagen::Y, nebulagen::Z>::apply_lighting(result);

		writer.write(result);
		return result;
	}
}

int main(int argc, char** argv)
{
	constexpr int seed = 4821903;
	nebulagen::nebula_t nebula = init_nebula("nebula.msgpack.gz", seed);

	glfwcontext r;
	//nebulascene s(nebula, r);
	particle_nebula_t pnebula(volume_to_particles(nebula.dust, seed), nebula.stars);
	nebulaparticlescene s(pnebula, r);

	r.run(argc, argv);
}
