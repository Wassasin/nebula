#pragma once

#include "nebula.hpp"

class particlelighting
{
private:
	particlelighting() = delete;
	particlelighting(particlelighting&) = delete;
	particlelighting& operator=(particlelighting) = delete;

public:
	static void apply_lighting(particle_nebula_t&);
	static void draw_debug();
};
