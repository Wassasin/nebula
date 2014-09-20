#pragma once

#include <vector>
#include <msgpack.hpp>

#include "volume.hpp"
#include "star.hpp"
#include "particle.hpp"

#include "gl/glm_msgpack.hpp"

template<size_t X, size_t Y, size_t Z>
struct volume_nebula_t
{
	volume<glm::vec4, X, Y, Z> dust;
	std::vector<star_t> stars;

	volume_nebula_t()
	: dust()
	, stars()
	{}

	volume_nebula_t(const std::vector<star_t>& _stars)
	: dust()
	, stars(_stars)
	{}

	volume_nebula_t(const volume<glm::vec4, X, Y, Z>& _dust, const std::vector<star_t>& _stars)
	: dust(_dust)
	, stars(_stars)
	{}

	MSGPACK_DEFINE(dust, stars)
};

struct particle_nebula_t
{
	std::vector<particle_t> particles;
	std::vector<star_t> stars;

	particle_nebula_t()
	: particles()
	, stars()
	{}

	particle_nebula_t(const std::vector<star_t>& _stars)
	: particles()
	, stars(_stars)
	{}

	particle_nebula_t(const std::vector<particle_t> _particles, const std::vector<star_t>& _stars)
	: particles(_particles)
	, stars(_stars)
	{}

	MSGPACK_DEFINE(particles, stars)
};
