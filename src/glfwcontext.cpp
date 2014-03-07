#include "glfwcontext.hpp"

#include <cmath>
#include <iostream>

#include <GL/glew.h>
#include <glfw3.h>

#include "gl.hpp"
#include "scope_guard.hpp"

glfwcontext::glfwcontext()
: rendercontext()
{}

void glfwcontext::run(int, char**)
{
	if(!glfwInit())
		throw std::runtime_error("Failed to initialize GLFW");

	scope_guard glfw_guard([]() { glfwTerminate(); });

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(size().first, size().second, "Nebula", NULL, NULL);
	if(window == NULL)
		throw std::runtime_error("Failed to open GLFW window. GPU not 3.3 compatible.");

	glfwMakeContextCurrent(window);

	glfwSetCursorPos(window, size().first/2, size().second/2);
	glfwSwapInterval(1);

	glewExperimental = true; // Needed for core profile
	gl::init();

	if(glGetError() == GL_INVALID_ENUM)
		std::cerr << "Swallowing false GL_INVALID_ENUM" << std::endl;
	else
		throw std::runtime_error("glGetError should have returned GL_INVALID_ENUM");

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	for(const callback_t& f : m_cbs[rcphase::init])
		f(*this);

	double lastTime = glfwGetTime();
	do
	{
		double currentTime = glfwGetTime();
		double delta = currentTime - lastTime;
		std::cerr << (1.0f / delta) << std::endl;
		lastTime = currentTime;

		for(const callback_t& f : m_cbs[rcphase::update])
			f(*this);

		for(const callback_t& f : m_cbs[rcphase::draw])
			f(*this);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	while(
		glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0
	);

	for(const callback_t& f : m_cbs[rcphase::cleanup])
		f(*this);
}
