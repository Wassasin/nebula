#pragma once

#include <vector>
#include <msgpack.hpp>

#include "volume.hpp"
#include "star.hpp"
#include "particle.hpp"

#include "util/glm_msgpack.hpp"

static inline void set_rgb(glm::uvec4& x, const glm::uvec3 color)
{
	x.r = color.r;
	x.g = color.g;
	x.b = color.b;
}

template<size_t X, size_t Y, size_t Z>
struct volume_nebula_t
{
	volume<glm::uvec4, X, Y, Z> dust;
	std::vector<star_t> stars;

	volume_nebula_t()
	: dust()
	, stars()
	{}

	volume_nebula_t(const std::vector<star_t>& _stars)
	: dust()
	, stars(_stars)
	{}

	MSGPACK_DEFINE(dust, stars)
};

struct particle_nebula_t
{
	std::vector<particle_t> particles;
	std::vector<star_t> stars;

	particle_nebula_t(const std::vector<star_t>& _stars)
	: particles()
	, stars(_stars)
	{}
};

const static size_t MAX_PARTICLE_PER_VOXEL = 10;

template<size_t X, size_t Y, size_t Z>
std::vector<particle_t> volume_to_particles(const volume<glm::uvec4, X, Y, Z>& dust, int seed)
{
	const static int mean = 100;

	std::default_random_engine engine(seed+1);
	std::poisson_distribution<int> dist(mean);

	const static GLfloat fX = X, fY = Y, fZ = Z;
	const static GLfloat fmean = mean;
	const static GLfloat fspread = 4.0f;

	std::vector<particle_t> particles;
	for(size_t x = 0; x < X; ++x)
		for(size_t y = 0; y < Y; ++y)
			for(size_t z = 0; z < Z; ++z)
			{
				const glm::uvec4& v = dust[glm::uvec3(x, y, z)];
				size_t particle_count = v.a / (255 / MAX_PARTICLE_PER_VOXEL);

				for(size_t i = 0; i < particle_count; i++)
				{
					GLfloat xdist = dist(engine) * (fspread / fmean) - (fspread - 0.5f);
					GLfloat ydist = dist(engine) * (fspread / fmean) - (fspread - 0.5f);
					GLfloat zdist = dist(engine) * (fspread / fmean) - (fspread - 0.5f);

					particles.emplace_back(particle_t({
						glm::vec3((x + xdist)/fX, (y + ydist)/fY, (z + zdist)/fZ),
						v,
						-1.0
					}));
				}
			}

	return particles;
}

