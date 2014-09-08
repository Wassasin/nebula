#include "nebulagen.hpp"

#include <iostream>
#include <cstdint>

#include "simplex.hpp"
#include "gl/glm_opts.hpp"

constexpr size_t nebulagen::SIZE;
constexpr GLfloat nebulagen::fX, nebulagen::fY, nebulagen::fZ;

static inline GLfloat exp_curve(const GLfloat x, const GLfloat cover, const GLfloat sharpness)
{
	GLfloat shift = x - (1.0f - cover);
	return 1.0f / (1.0f + glm::exp(-1.0f * shift * sharpness));
}

static inline GLfloat sine_point(const simplex& s, glm::vec3 pos, glm::vec3 periods, GLfloat turb_power, size_t turb_size)
{
	GLfloat turb = turb_power * s.octave_noise(turb_size, 0.25f, 1.0f, pos);
	GLfloat v = pos.x * periods.x + pos.y * periods.y + pos.z * periods.z + turb;
	return glm::clamp(glm::sin(v * (GLfloat)M_PI), 0.0f, 1.0f);
}

void nebulagen::generate_cloud(const glm::vec3 fcenter, const GLfloat size, const GLfloat noise_mod, volume<GLfloat, X, Y, Z>& density_volume)
{
	simplex s(m_seed);

	glm::vec3 fstart = fcenter - glm::vec3(0.5)*size;
	glm::vec3 fend = fcenter + glm::vec3(0.5)*size;

	glm::ivec3 start = iupcast(fstart, fX, fY, fZ);
	glm::ivec3 end = iupcast(fend, fX, fY, fZ);

	for(size_t x = glm::max(start.x, 0); x < (size_t)glm::min((int)X, end.x); ++x)
		for(size_t y = glm::max(start.y, 0); y < (size_t)glm::min((int)Y, end.y); ++y)
			for(size_t z = glm::max(start.z, 0); z < (size_t)glm::min((int)Z, end.z); ++z)
			{
				glm::uvec3 pos(x, y, z);
				glm::vec3 fpos = downcast(pos, fX, fY, fZ);

				glm::vec3 objfpos = (fpos - fstart) / size;

				glm::vec3 orb = glm::sin(objfpos*(GLfloat)M_PI);

				GLuint density = (
					255.0 *
					exp_curve(
						glm::clamp(
							(
								(GLfloat) std::pow(orb.x * orb.y * orb.z, 2.0) +
								s.octave_noise(5.0f, 0.6f, 1.0f, fpos + glm::vec3(noise_mod))
							),
							0.5f,
							1.5f
						) - 0.5f,
						0.4f,
						40.0f
					)
				);

				density_volume[pos] = glm::clamp(
					(uint16_t)(density + density_volume[pos]),
					(uint16_t) 0,
					(uint16_t) 255
				);
			}
}

std::vector<star_t> nebulagen::generate_stars()
{
	return {
		{glm::vec3(0.8, 0.5, 0.2), glm::uvec3(240, 240, 220)},
		{glm::vec3(0.3, 0.8, 0.2), glm::uvec3(110, 100, 255)},
		{glm::vec3(0.2, 0.4, 0.8), glm::uvec3(50, 50, 255)}
	};
}

volume<glm::uvec4, nebulagen::X, nebulagen::Y, nebulagen::Z> nebulagen::generate_dust()
{
	simplex s(m_seed);
	std::default_random_engine engine(m_seed+1);

	static const glm::uvec3 brownish(255, 222, 150);
	static const glm::uvec3 blackish(10, 1, 1);

	std::cerr << "Seeding dust" << std::endl;

	static std::uniform_real_distribution<GLfloat> antiedge_dist(0.2, 0.8);
	static std::uniform_real_distribution<GLfloat> size_dist(0.5, 1.0);
	static std::uniform_real_distribution<GLfloat> small_size_dist(0.2, 0.5);

	volume<GLfloat, X, Y, Z> reflective_volume;
	for(size_t i = 0; i < 20; ++i)
	{
		glm::vec3 fcenter(antiedge_dist(engine), antiedge_dist(engine), antiedge_dist(engine));
		generate_cloud(fcenter, size_dist(engine), i, reflective_volume);
	}

	volume<GLfloat, X, Y, Z> absorbant_volume;
	for(size_t i = 0; i < 20; ++i)
	{
		glm::vec3 fcenter(antiedge_dist(engine), antiedge_dist(engine), antiedge_dist(engine));
		generate_cloud(fcenter, small_size_dist(engine), i+20, absorbant_volume);
	}

	std::cerr << "Drawing dust" << std::endl;

	volume<glm::uvec4, X, Y, Z> dust_volume;

	for(size_t x = 0; x < X; ++x)
		for(size_t y = 0; y < Y; ++y)
			for(size_t z = 0; z < Z; ++z)
			{
				constexpr GLfloat division = 0.75;

				glm::uvec3 pos(x, y, z);

				GLfloat absorbant = absorbant_volume[pos] * division;
				GLfloat reflective = reflective_volume[pos] * (1.0 - division);

				GLfloat density = absorbant + reflective;

				dust_volume[pos].a = 255 * density;
				set_rgb(
					dust_volume[pos],
					upcast(glm::mix(downcast(brownish), downcast(blackish), absorbant / density))
				);
			}

	return dust_volume;
}

nebulagen::nebula_t nebulagen::generate()
{
	return nebula_t(generate_dust(), generate_stars());
}
