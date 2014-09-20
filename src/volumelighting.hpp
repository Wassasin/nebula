#pragma once

#include <vector>

#include "nebula.hpp"

#include "gl/glm_opts.hpp"

template<size_t X, size_t Y, size_t Z>
class volumelighting
{
	static constexpr GLfloat fX = X, fY = Y, fZ = Z;

	static GLfloat raycast_stars(const std::vector<star_t>& nebula_stars, volume<glm::vec3, X, Y, Z>& light_volume, const volume<glm::vec4, X, Y, Z>& dust_volume)
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
					glm::vec3 fpos = downcast(pos, fX, fY, fZ);

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
							glm::uvec3 uvec = upcast(vec, fX, fY, fZ);
							intensity -= dust_volume[uvec].a * occlusion * stepsize;

							vec += delta_dir;

							if(intensity <= 0.0)
								break;
						}

						intensity *= 1.0f - std::pow(len*falloff, 2.0f);

						if(intensity > 0.0) // If not completely occluded
						{
							color += star.color * intensity;
							total_intensity += intensity;
						}
					}

					light_volume[pos] = color;
					max_intensity = glm::max(max_intensity, total_intensity);
				}

		return max_intensity;
	}

	static void apply_mockup_to_dust(volume<glm::uvec4, X, Y, Z>& nebula_dust, const volume<glm::vec4, X, Y, Z>& dust_volume)
	{
		for(size_t x = 0; x < X; ++x)
			for(size_t y = 0; y < Y; ++y)
				for(size_t z = 0; z < Z; ++z)
				{
					glm::uvec3 pos(x, y, z);
					nebula_dust[pos] = glm::vec4(x/fX, y/fY, z/fZ, glm::clamp(dust_volume[pos].a, 0.0f, 1.0f));
				}
	}

	static void apply_lighting_to_dust(volume<glm::vec4, X, Y, Z>& nebula_dust, const volume<glm::vec3, X, Y, Z>& light_volume, const volume<glm::vec4, X, Y, Z>& dust_volume, const GLfloat intensity_multiplier)
	{
		for(size_t x = 0; x < X; ++x)
			for(size_t y = 0; y < Y; ++y)
				for(size_t z = 0; z < Z; ++z)
				{
					glm::uvec3 pos(x, y, z);

					glm::vec3 color_tmp = (light_volume[pos] * intensity_multiplier) * dust_volume[pos].rgb();
					set_rgb(nebula_dust[pos], color_tmp);
					nebula_dust[pos].a = glm::clamp(dust_volume[pos].a, 0.0f, 1.0f);
				}
	}

public:
	static void apply_lighting(volume_nebula_t<X, Y, Z>& n)
	{
		std::cerr << "Raycasting stars" << std::endl;

		volume<glm::vec3, X, Y, Z> light_volume;
		GLfloat max_intensity = raycast_stars(n.stars, light_volume, n.dust);

		std::cerr << "Applying lighting" << std::endl;
		apply_lighting_to_dust(n.dust, light_volume, n.dust, max_intensity / ((GLfloat) n.stars.size()));
	}
};
