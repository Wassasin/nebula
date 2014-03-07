#include "glutcontext.hpp"

#include <iostream>

#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "gl.hpp"

glutcontext* glutcontext::m_singleton;

glutcontext::glutcontext()
: rendercontext()
{}

void glutcontext::run(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(size().first, size().second);
	glutCreateWindow("nebula");

	m_singleton = this;
	glutDisplayFunc(&glutcontext::draw);

	glewExperimental = true; // Needed for core profile
	gl::init();

	for(const callback_t& f : m_cbs[rcphase::init])
		f(*this);

	glutMainLoop();

	for(const callback_t& f : m_cbs[rcphase::cleanup])
		f(*this);
}

void glutcontext::draw()
{
	for(const callback_t& f : m_singleton->m_cbs[rcphase::update])
		f(*m_singleton);

	for(const callback_t& f : m_singleton->m_cbs[rcphase::draw])
		f(*m_singleton);

	glutSwapBuffers();
	glutPostRedisplay();
}
