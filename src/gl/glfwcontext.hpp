#pragma once

#include <GL/glew.h>
#include <glfw3.h>

#include "rendercontext.hpp"

class glfwcontext : public rendercontext
{
private:
	void process_input(float delta, GLFWwindow* window);

public:
	glfwcontext();
	void run(int argc, char** argv);
};
