#pragma once

#include "volume.hpp"
#include "particle.hpp"

template<size_t X, size_t Y, size_t Z>
std::vector<particle_t> volume_to_particles(const volume<glm::vec4, X, Y, Z>& dust, int seed, size_t budget = 500000)
{
	std::cerr << "Instancing particles" << std::endl;
	const static int mean = 100;

	std::default_random_engine engine(seed+1);
	std::poisson_distribution<int> dist(mean);

	const static GLfloat fX = X, fY = Y, fZ = Z;
	const static GLfloat fmean = mean;
	const static GLfloat fspread = 4.0f;

	std::vector<particle_t> particles;
	GLfloat alpha_sum = 0.0f;
	for(size_t x = 0; x < X; ++x)
		for(size_t y = 0; y < Y; ++y)
			for(size_t z = 0; z < Z; ++z)
				alpha_sum += dust[glm::uvec3(x, y, z)].a;

	GLfloat max_particle_per_voxel = budget / alpha_sum;
	std::cout << "Instancing " << max_particle_per_voxel << " particles per voxel (" << X << "x" << Y << "x" << Z << ")" << std::endl;

	for(size_t x = 0; x < X; ++x)
		for(size_t y = 0; y < Y; ++y)
			for(size_t z = 0; z < Z; ++z)
			{
				const glm::vec4 v = dust[glm::uvec3(x, y, z)];
				size_t particle_count = v.a * max_particle_per_voxel + 0.5f;

				assert(v.a >= 0.0f && v.a <= 1.0f);

				for(size_t i = 0; i < particle_count; i++)
				{
					GLfloat xdist = dist(engine) * (fspread / fmean) - (fspread - 0.5f);
					GLfloat ydist = dist(engine) * (fspread / fmean) - (fspread - 0.5f);
					GLfloat zdist = dist(engine) * (fspread / fmean) - (fspread - 0.5f);

					particles.emplace_back(particle_t({
						glm::vec3((x + xdist)/fX, (y + ydist)/fY, (z + zdist)/fZ),
						v
					}));
				}
			}

	return particles;
}
