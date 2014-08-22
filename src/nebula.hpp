#pragma once

#include <vector>
#include <msgpack.hpp>

#include "volume.hpp"
#include "star.hpp"

#include "util/glm_msgpack.hpp"

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
