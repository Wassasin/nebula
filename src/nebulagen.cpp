#include "nebulagen.hpp"

#include "simplex.hpp"

#include <iostream>
#include <cstdint>

constexpr size_t nebulagen::SIZE;
constexpr GLfloat nebulagen::fX, nebulagen::fY, nebulagen::fZ;

static inline void set_rgb(glm::uvec4& x, const glm::uvec3 color)
{
	x.r = color.r;
	x.g = color.g;
	x.b = color.b;
}

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

static inline glm::vec3 downcast(const glm::uvec3 p, uint8_t fX = nebulagen::fX, uint8_t fY = nebulagen::fY, uint8_t fZ = nebulagen::fZ)
{
	return glm::vec3((GLfloat)p.x / fX, (GLfloat)p.y / fY, (GLfloat)p.z / fZ);
}

static inline glm::uvec3 upcast(const glm::vec3 p, uint8_t fX = nebulagen::fX, uint8_t fY = nebulagen::fY, uint8_t fZ = nebulagen::fZ)
{
	glm::vec3 tmp = glm::round(p * glm::vec3(fX, fY, fZ));
	return glm::clamp(glm::uvec3(tmp.x, tmp.y, tmp.z), glm::uvec3(0), glm::uvec3(255));
}

static inline glm::ivec3 iupcast(const glm::vec3 p, uint8_t fX = nebulagen::fX, uint8_t fY = nebulagen::fY, uint8_t fZ = nebulagen::fZ)
{
	glm::vec3 tmp = glm::round(p * glm::vec3(fX, fY, fZ));
	return glm::clamp(glm::ivec3(tmp.x, tmp.y, tmp.z), glm::ivec3(-255), glm::ivec3(255));
}

GLfloat nebulagen::raycast_stars(const std::vector<star_t>& nebula_stars, volume<glm::vec3, X, Y, Z>& light_volume, const volume<glm::uvec4, X, Y, Z>& dust_volume)
{
	static constexpr GLfloat occlusion = 0.005;
	static constexpr GLfloat stepsize = 0.001;
	static constexpr GLfloat falloff = 1.2;

	GLfloat max_intensity = 0.0;

	for(size_t x = 0; x < X; ++x)
		for(size_t y = 0; y < Y; ++y)
			for(size_t z = 0; z < Z; ++z)
			{
				glm::uvec3 pos(x, y, z);
				glm::vec3 fpos = downcast(pos);

				glm::vec3 color = glm::vec3(0.0);
				GLfloat total_intensity = 0.0;

				for(const star_t& star : nebula_stars)
				{
					glm::vec3 dir = fpos - star.pos;
					GLfloat len = glm::length(dir);

					if(len == 0.0)
						continue;

					glm::vec3 norm_dir = glm::normalize(dir);
					glm::vec3 delta_dir = norm_dir * stepsize;
					GLfloat delta_dir_len = glm::length(delta_dir);

					glm::vec3 vec = star.pos;

					size_t step_count = len / delta_dir_len - 1;

					GLfloat intensity = 1.0;
					for(size_t i = 0; i < step_count; ++i)
					{
						glm::uvec3 uvec = upcast(vec);
						intensity -= dust_volume[uvec].a * occlusion * stepsize;

						vec += delta_dir;

						if(intensity <= 0.0)
							break;
					}

					intensity *= 1.0f - std::pow(len*falloff, 2.0f);

					if(intensity > 0.0) // If not completely occluded
					{
						color += downcast(star.color, 255, 255, 255) * intensity;
						total_intensity += intensity;
					}
				}

				light_volume[pos] = color;
				max_intensity = glm::max(max_intensity, total_intensity);
			}

	return max_intensity;
}

void nebulagen::apply_mockup_to_dust(volume<glm::uvec4, X, Y, Z>& nebula_dust, const volume<glm::uvec4, X, Y, Z>& dust_volume)
{
	for(size_t x = 0; x < X; ++x)
		for(size_t y = 0; y < Y; ++y)
			for(size_t z = 0; z < Z; ++z)
			{
				glm::uvec3 pos(x, y, z);
				nebula_dust[pos] = glm::uvec4(x, y, z, glm::clamp((int)dust_volume[pos].a, 0, 255));
			}
}

void nebulagen::apply_lighting_to_dust(volume<glm::uvec4, X, Y, Z>& nebula_dust, const volume<glm::vec3, X, Y, Z>& light_volume, const volume<glm::uvec4, X, Y, Z>& dust_volume, const GLfloat intensity_multiplier)
{
	for(size_t x = 0; x < X; ++x)
		for(size_t y = 0; y < Y; ++y)
			for(size_t z = 0; z < Z; ++z)
			{
				glm::uvec3 pos(x, y, z);

				glm::vec3 color_tmp = (light_volume[pos] * intensity_multiplier) * downcast(dust_volume[pos].rgb(), 255, 255, 255);
				set_rgb(nebula_dust[pos], upcast(color_tmp, 255, 255, 255));
				nebula_dust[pos].a = glm::clamp((int)dust_volume[pos].a, 0, 255);
			}
}

void nebulagen::generate_cloud(const glm::vec3 fcenter, const GLfloat size, const GLfloat noise_mod, volume<GLfloat, X, Y, Z>& density_volume)
{
	simplex s(m_seed);

	glm::vec3 fstart = fcenter - glm::vec3(0.5)*size;
	glm::vec3 fend = fcenter + glm::vec3(0.5)*size;

	glm::ivec3 start = iupcast(fstart);
	glm::ivec3 end = iupcast(fend);

	for(size_t x = glm::max(start.x, 0); x < (size_t)glm::min((int)X, end.x); ++x)
		for(size_t y = glm::max(start.y, 0); y < (size_t)glm::min((int)Y, end.y); ++y)
			for(size_t z = glm::max(start.z, 0); z < (size_t)glm::min((int)Z, end.z); ++z)
			{
				glm::uvec3 pos(x, y, z);
				glm::vec3 fpos = downcast(pos);

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

nebulagen::nebula_t nebulagen::generate()
{
	simplex s(m_seed);
	std::default_random_engine engine(m_seed+1);

	nebula_t result({
		{glm::uvec3(240, 240, 220), glm::vec3(0.8, 0.5, 0.2)},
		{glm::uvec3(110, 100, 255), glm::vec3(0.3, 0.8, 0.2)},
		{glm::uvec3(50, 50, 255), glm::vec3(0.2, 0.4, 0.8)}
	});

	auto& nebula_dust = result.dust;
	const auto& nebula_stars = result.stars;

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
				set_rgb(dust_volume[pos], upcast(glm::mix(
					downcast(brownish, 255, 255, 255),
					downcast(blackish, 255, 255, 255),
					absorbant / density
				), 255, 255, 255));
			}

	/*for(size_t x = 0; x < X; ++x)
		for(size_t y = 0; y < Y; ++y)
			for(size_t z = 0; z < Z; ++z)
			{
				glm::uvec3 pos(x, y, z);
				glm::vec3 fpos = downcast(pos);

				//uint8_t density = 255 * exp_curve(sine_point(s, fpos*4.0f, glm::vec3(0.2f, 1.0f, 0.2f), 1.0f, 4), 0.45, 20.0f);
				//uint8_t density = 255 * s.octave_noise(20.0f, 0.25f, 1.0f, fpos);

				glm::vec3 tmp = glm::sin(fpos*(GLfloat)M_PI);
				uint8_t density = 255.0 *
						exp_curve(glm::clamp((
							(GLfloat)std::pow(tmp.x * tmp.y * tmp.z, 2.0) +
							s.octave_noise(10.0f, 0.5f, 1.0f, fpos+glm::vec3(3.0f))
						), 0.5f, 1.5f)-0.5f, 0.5f, 40.0f);
				dust_volume[pos] = glm::uvec4(brownish.r, brownish.g, brownish.b, density);
			}*/

	std::cerr << "Raycasting stars" << std::endl;

	volume<glm::vec3, X, Y, Z> light_volume;
	GLfloat max_intensity = raycast_stars(nebula_stars, light_volume, dust_volume);

	std::cerr << "Applying lighting" << std::endl;
	//nebula_dust = dust_volume;
	//apply_mockup_to_dust(nebula_dust, dust_volume);
	apply_lighting_to_dust(nebula_dust, light_volume, dust_volume, max_intensity / ((GLfloat) nebula_stars.size()));

	return result;
}
