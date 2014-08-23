#pragma once

#include <iostream>
#include <cmath>
#include <random>
#include <algorithm>

#include <glm/glm.hpp>

class perlin {
	std::vector<int> m_p;

public:
	perlin(unsigned int seed)
	{
		m_p.resize(256);
		std::iota(m_p.begin(), m_p.end(), 0);

		std::default_random_engine engine(seed);
		std::shuffle(m_p.begin(), m_p.end(), engine);

		// Duplicate the permutation vector
		m_p.insert(m_p.end(), m_p.begin(), m_p.end());
	}

	double noise(glm::vec3 pos) const
	{
		GLfloat& x = pos[0];
		GLfloat& y = pos[1];
		GLfloat& z = pos[2];

		int X = (int) floor(x) & 255;
		int Y = (int) floor(y) & 255;
		int Z = (int) floor(z) & 255;

		x -= floor(x);
		y -= floor(y);
		z -= floor(z);

		GLfloat u = fade(x);
		GLfloat v = fade(y);
		GLfloat w = fade(z);

		int A = m_p[X] + Y;
		int AA = m_p[A] + Z;
		int AB = m_p[A + 1] + Z;
		int B = m_p[X + 1] + Y;
		int BA = m_p[B] + Z;
		int BB = m_p[B + 1] + Z;

		GLfloat res = lerp(w,
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
	GLfloat fade(GLfloat t) const
	{
		return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
	}

	GLfloat lerp(GLfloat t, GLfloat a, GLfloat b) const
	{
		return a + t * (b - a);
	}

	GLfloat grad(int hash, GLfloat x, GLfloat y, GLfloat z) const
	{
		int h = hash & 15;

		GLfloat u = h < 8 ? x : y,
			   v = h < 4 ? y : h == 12 || h == 14 ? x : z;

		return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
	}
};
