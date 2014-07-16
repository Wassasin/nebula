#pragma once

#include <GL/gl.h>

#include "volume.hpp"

class nebulagen
{
public:
	static constexpr size_t SIZE = 64;
	static constexpr size_t X = SIZE, Y = SIZE, Z = SIZE;
	static constexpr GLfloat fX = X, fY = Y, fZ = Z;

	struct star_t
	{
		glm::uvec3 color;
		glm::vec3 pos;
	};

	struct nebula_t
	{
		volume<glm::uvec4, X, Y, Z> dust;
		std::vector<star_t> stars;

		nebula_t(const std::vector<star_t>& _stars)
		: dust()
		, stars(_stars)
		{}
	};

private:
	unsigned int m_seed;

	static GLfloat raycast_stars(const std::vector<star_t>& stars, volume<glm::vec3, X, Y, Z>& light_volume, const volume<glm::uvec4, X, Y, Z>& dust_volume);

	static void apply_mockup_to_dust(volume<glm::uvec4, X, Y, Z>& nebula_dust, const volume<glm::uvec4, X, Y, Z>& dust_volume);
	static void apply_lighting_to_dust(volume<glm::uvec4, X, Y, Z>& nebula_dust, const volume<glm::vec3, X, Y, Z>& light_volume, const volume<glm::uvec4, X, Y, Z>& dust_volume, const GLfloat intensity_multiplier);
	void generate_cloud(const glm::vec3 fcenter, const GLfloat size, const GLfloat noise_mod, volume<GLfloat, X, Y, Z>& density_volume);

public:
	nebulagen(unsigned int seed)
	: m_seed(seed)
	{}

	nebula_t generate();
};
