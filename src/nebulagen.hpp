#pragma once

#include <GL/gl.h>

#include "perlin.hpp"
#include "volume.hpp"

template<size_t SIZE>
class nebulagen
{
public:
	static constexpr size_t X = SIZE, Y = SIZE, Z = SIZE;
	static constexpr GLfloat fX = X, fY = Y, fZ = Z;

	typedef volume<glm::vec4, X, Y, Z> volume_t;

private:
	unsigned int m_seed;

	static inline GLfloat exp_curve(const GLfloat x, const GLfloat cover, const GLfloat sharpness)
	{
		GLfloat shift = x - (1.0f - cover);
		return 1.0f / (1.0f + glm::exp(-1.0f * shift * sharpness));
	}

	static inline GLfloat octave_point(const perlin& p, size_t octaves, glm::vec3 pos)
	{
		GLfloat result = 0.0f;

		for(size_t i = 0; i < octaves; ++i)
		{
			result += p.noise(pos) / glm::pow(2.0f, (GLfloat)i+1.0f);
			pos *= 2.0f;
		}

		return result;
	}

	static inline GLfloat sine_point(const perlin& p, glm::vec3 pos, glm::vec3 periods, GLfloat turb_power, size_t turb_size)
	{
		GLfloat turb = turb_power * octave_point(p, turb_size, pos);
		GLfloat v = pos.x * periods.x + pos.y * periods.y + pos.z * periods.z + turb;
		return glm::clamp(glm::sin(v * 3.14159f), 0.0f, 1.0f);
	}

	static inline glm::vec3 downcast(const glm::uvec3 p)
	{
		return glm::vec3((GLfloat)p.x / fX, (GLfloat)p.y / fY, (GLfloat)p.z / fZ);
	}

public:
	nebulagen(unsigned int seed)
	: m_seed(seed)
	{}

	volume_t generate()
	{
		perlin p(m_seed);

		volume_t result;

		for(size_t x = 0; x < X; ++x)
			for(size_t y = 0; y < Y; ++y)
				for(size_t z = 0; z < Z; ++z)
				{
					glm::uvec3 pos(x, y, z);
					glm::vec3 fpos = downcast(pos);

					glm::vec4& v = result[pos];

					v.r = fpos.x;
					v.g = fpos.y;
					v.b = fpos.z;
					v.a = glm::clamp(exp_curve(octave_point(p, 4, fpos*8.0f), 0.5f, 50.0f), 0.0f, 1.0f);
					//v.a = exp_curve(sine_point(p, fpos*2.0f, glm::vec3(0.2f, 1.0f, 0.2f), 5.0f, 16), 0.5, 10.0f);
				}

		return result;
	}
};
