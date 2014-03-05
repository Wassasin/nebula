#pragma once

#include <iostream>
#include <cmath>
#include <random>
#include <algorithm>

#include "defines.hpp"

class perlin {
	std::vector<qsint> m_p;

public:
	perlin(quint seed)
	{
		m_p.resize(256);
		std::iota(m_p.begin(), m_p.end(), 0);

		std::default_random_engine engine(seed);
		std::shuffle(m_p.begin(), m_p.end(), engine);

		// Duplicate the permutation vector
		m_p.insert(m_p.end(), m_p.begin(), m_p.end());
	}

	double noise(qfloat x, qfloat y, qfloat z)
	{
		qsint X = (qsint) floor(x) & 255;
		qsint Y = (qsint) floor(y) & 255;
		qsint Z = (qsint) floor(z) & 255;

		x -= floor(x);
		y -= floor(y);
		z -= floor(z);

		qfloat u = fade(x);
		qfloat v = fade(y);
		qfloat w = fade(z);

		qsint A = m_p[X] + Y;
		qsint AA = m_p[A] + Z;
		qsint AB = m_p[A + 1] + Z;
		qsint B = m_p[X + 1] + Y;
		qsint BA = m_p[B] + Z;
		qsint BB = m_p[B + 1] + Z;

		qfloat res = lerp(w,
						  lerp(v,
							   lerp(u,
									grad(m_p[AA], x, y, z),
									grad(m_p[BA], x-1, y, z)),
							   lerp(u,
									grad(m_p[AB], x, y-1, z),
									grad(m_p[BB], x-1, y-1, z))),
						  lerp(v,
							   lerp(u,
									grad(m_p[AA+1], x, y, z-1),
									grad(m_p[BA+1], x-1, y, z-1)),
							   lerp(u,
									grad(m_p[AB+1], x, y-1, z-1),
									grad(m_p[BB+1], x-1, y-1, z-1))));

		return (res + 1.0)/2.0;
	}

private:
	qfloat fade(qfloat t)
	{
		return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
	}

	qfloat lerp(qfloat t, qfloat a, qfloat b)
	{
		return a + t * (b - a);
	}

	qfloat grad(qsint hash, qfloat x, qfloat y, qfloat z)
	{
		int h = hash & 15;

		qfloat u = h < 8 ? x : y,
			   v = h < 4 ? y : h == 12 || h == 14 ? x : z;

		return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
	}
};
