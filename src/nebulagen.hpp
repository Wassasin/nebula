#pragma once

#include "nebula.hpp"
#include "volume.hpp"
#include "star.hpp"

class nebulagen
{
public:
	static constexpr size_t SIZE = 128;
	static constexpr size_t X = SIZE, Y = SIZE, Z = SIZE;
	static constexpr GLfloat fX = X, fY = Y, fZ = Z;

	typedef volume_nebula_t<X, Y, Z> nebula_t;

private:
	unsigned int m_seed;

	static std::vector<star_t> generate_stars();
	volume<glm::uvec4, X, Y, Z> generate_dust();

	void generate_cloud(const glm::vec3 fcenter, const GLfloat size, const GLfloat noise_mod, volume<GLfloat, X, Y, Z>& density_volume);

public:
	nebulagen(unsigned int seed)
	: m_seed(seed)
	{}

	volume_nebula_t<X, Y, Z> generate();
};
