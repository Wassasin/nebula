#include "rendercontext.hpp"
#include "simplescene.hpp"

int main(int argc, char** argv)
{
	rendercontext r;
	simplescene s(r); // Context for scene

	/*r.add_cb(rcphase::draw, [](rendercontext& s) {
			glLineWidth(2.0);
			glBegin(GL_LINES);
			for(qi i = 0; i < 3; i++)
			{
				glColor3f(i == 0, i == 1, i == 2);
				glVertex3f(0.0, 0.0, 0.0);
				glVertex3f(i == 0, i == 1, i == 2);
			}
			glEnd();
		});*/

	r.run();
}
