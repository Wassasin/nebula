#pragma once

#include <memory>
#include <boost/optional.hpp>

#include "glm_include.hpp"
#include "gl.hpp"

#include "texture.hpp"

class textureatlas
{
public:
	static const size_t texture_size = 1024;
	const size_t tile_count;
	const size_t tile_sqrtcount; // sqrt(tile_count)

private:
	size_t texture_count; // tile_count actually used at the moment
	std::unique_ptr<GLubyte[]> data;
	boost::optional<GLuint> texture_id;

public:
	textureatlas(const size_t tile_count)
	: tile_count(tile_count)
	, tile_sqrtcount(std::sqrt(tile_count))
	, texture_count(0)
	, data(new GLubyte[texture_size * texture_size * tile_sqrtcount * tile_sqrtcount * 4])
	, texture_id()
	{
		assert(std::pow(2.0f, std::log2(texture_size)) == texture_size); // 2^x-factor
		assert(std::pow(2.0f, std::log2(tile_sqrtcount)) == tile_sqrtcount); // 2^x-factor
		assert(std::pow(tile_sqrtcount, 2.0f) == tile_count);

		for(size_t y = 0; y < texture_size * tile_sqrtcount; ++y)
			for(size_t x = 0; x < texture_size * tile_sqrtcount; ++x)
				for(size_t i = 0; i < 4; ++i)
					data[y * texture_size * tile_sqrtcount * 4 + x * 4 + i] = 0;
	}

	void bind()
	{
		assert(!texture_id);

		GLuint tid;
		glGenTextures(1, &tid);
		glBindTexture(GL_TEXTURE_2D, tid);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tile_sqrtcount*texture_size, tile_sqrtcount*texture_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.get());

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);

		texture_id = tid;
	}

	size_t add_texture(GLubyte* texture)
	{
		assert(texture_count < tile_count);
		const size_t i = texture_count++;

		const glm::uvec2 tile = get_tilecoords(i);
		for(size_t y = 0; y < texture_size; ++y)
		{
			size_t datay = ((size_t)tile.y) * texture_size + y;
			for(size_t x = 0; x < texture_size; ++x)
			{
				size_t datax = ((size_t)tile.x) * texture_size + x;
				for(size_t i = 0; i < 4; ++i)
					data[datay * texture_size * tile_sqrtcount * 4 + datax * 4 + i] = texture[y*texture_size*4 + x*4 + i];
			}
		}

		return i;
	}

	size_t add_texture(const texture& texture)
	{
		assert(texture.size == texture_size);
		return add_texture(texture.data.get());
	}

	glm::uvec2 get_tilecoords(size_t i) const
	{
		assert(i < texture_count);
		return glm::uvec2(i % tile_sqrtcount, i / tile_sqrtcount);
	}

	GLfloat get_fractionsize() const
	{
		return 1.0f / (GLfloat)tile_sqrtcount;
	}

	glm::vec2 get_fractionoffset(size_t i) const
	{
		GLfloat fs = get_fractionsize();
		glm::uvec2 tc = get_tilecoords(i);
		return glm::vec2((GLfloat)tc.x * fs, (GLfloat)tc.y * fs);
	}

	GLuint get_texture_id() const
	{
		return texture_id.get();
	}
};
