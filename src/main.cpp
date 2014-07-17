#include "glfwcontext.hpp"
#include "glutcontext.hpp"
#include "simplescene.hpp"
#include "nebulascene.hpp"
#include "nebulaparticlescene.hpp"

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

int main(int argc, char** argv)
{
	glfwcontext r;
	nebulaparticlescene s(r);

	r.run(argc, argv);
}
