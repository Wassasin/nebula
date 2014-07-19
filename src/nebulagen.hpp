#pragma once

#include <GL/gl.h>
#include <msgpack.hpp>

#include "volume.hpp"

namespace msgpack {

    inline glm::uvec3& operator>>(object o, glm::uvec3& t)
    {
        uint8_t x, y, z;

        if(o.type != type::ARRAY) { throw type_error(); }
        if(o.via.array.size != 3) { throw type_error(); }
        o.via.array.ptr[0].convert(&x);
        o.via.array.ptr[1].convert(&y);
        o.via.array.ptr[2].convert(&z);

        t = glm::uvec3(x, y, z);

        return t;
    }

    template <typename Stream>
    inline packer<Stream>& operator<<(packer<Stream>& o, const glm::uvec3& x)
    {
        o.pack_array(3);
        o.pack((uint8_t)x.x);
        o.pack((uint8_t)x.y);
        o.pack((uint8_t)x.z);
        return o;
    }

    inline glm::uvec4& operator>>(object o, glm::uvec4& x)
    {
        uint8_t r, g, b, a;

        if(o.type != type::ARRAY) { throw type_error(); }
        if(o.via.array.size != 4) { throw type_error(); }
        o.via.array.ptr[0].convert(&r);
        o.via.array.ptr[1].convert(&g);
        o.via.array.ptr[2].convert(&b);
        o.via.array.ptr[3].convert(&a);

        x = glm::uvec4(r, g, b, a);

        return x;
    }

    template <typename Stream>
    inline packer<Stream>& operator<<(packer<Stream>& o, const glm::uvec4& x)
    {
        o.pack_array(4);
        o.pack((uint8_t)x.r);
        o.pack((uint8_t)x.g);
        o.pack((uint8_t)x.b);
        o.pack((uint8_t)x.a);
        return o;
    }

    inline glm::vec3& operator>>(object o, glm::vec3& t)
    {
        GLfloat x, y, z;

        if(o.type != type::ARRAY) { throw type_error(); }
        if(o.via.array.size != 3) { throw type_error(); }
        o.via.array.ptr[0].convert(&x);
        o.via.array.ptr[1].convert(&y);
        o.via.array.ptr[2].convert(&z);

        t = glm::vec3(x, y, z);

        return t;
    }

    template <typename Stream>
    inline packer<Stream>& operator<<(packer<Stream>& o, const glm::vec3& x)
    {
        o.pack_array(3);
        o.pack((GLfloat)x.x);
        o.pack((GLfloat)x.y);
        o.pack((GLfloat)x.z);
        return o;
    }

}

class nebulagen
{
public:
	static constexpr size_t SIZE = 128;
	static constexpr size_t X = SIZE, Y = SIZE, Z = SIZE;
	static constexpr GLfloat fX = X, fY = Y, fZ = Z;

	struct star_t
	{
		glm::uvec3 color;
		glm::vec3 pos;

        MSGPACK_DEFINE(color, pos)
	};

	struct nebula_t
	{
		volume<glm::uvec4, X, Y, Z> dust;
		std::vector<star_t> stars;

		nebula_t()
		: dust()
		, stars()
		{}

		nebula_t(const std::vector<star_t>& _stars)
		: dust()
		, stars(_stars)
		{}

        MSGPACK_DEFINE(dust, stars)
	};

private:
	unsigned int m_seed;

	static GLfloat raycast_stars(const std::vector<star_t>& stars, volume<glm::vec3, X, Y, Z>& light_volume, const volume<glm::uvec4, X, Y, Z>& dust_volume);

	static void apply_mockup_to_dust(volume<glm::uvec4, X, Y, Z>& nebula_dust, const volume<glm::uvec4, X, Y, Z>& dust_volume);
	static void apply_lighting_to_dust(volume<glm::uvec4, X, Y, Z>& nebula_dust, const volume<glm::vec3, X, Y, Z>& light_volume, const volume<glm::uvec4, X, Y, Z>& dust_volume, const GLfloat intensity_multiplier);
	void generate_cloud(const glm::vec3 fcenter, const GLfloat size, const GLfloat noise_mod, volume<GLfloat, X, Y, Z>& density_volume);

public:
	nebulagen(unsigned int seed)
	: m_seed(seed)
	{}

	nebula_t generate();
};
