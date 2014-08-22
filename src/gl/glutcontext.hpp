#pragma once

#include "rendercontext.hpp"

class glutcontext : public rendercontext
{
private:
	static void draw();
	static glutcontext* m_singleton;

public:
	glutcontext();
	void run(int argc, char** argv);
};
