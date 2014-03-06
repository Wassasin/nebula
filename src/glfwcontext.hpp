#pragma once

#include "rendercontext.hpp"

class glfwcontext : public rendercontext
{
public:
	glfwcontext();
	void run();
};
