#pragma once

#include <msgpack.hpp>

#include "../gl/gl.hpp"
#include "glm_include.hpp"

namespace msgpack {

	inline glm::uvec3& operator>>(object o, glm::uvec3& t)
	{
		uint32_t x, y, z;

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
		o.pack((uint32_t)x.x);
		o.pack((uint32_t)x.y);
		o.pack((uint32_t)x.z);
		return o;
	}

	inline glm::uvec4& operator>>(object o, glm::uvec4& x)
	{
		uint32_t r, g, b, a;

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
		o.pack((uint32_t)x.r);
		o.pack((uint32_t)x.g);
		o.pack((uint32_t)x.b);
		o.pack((uint32_t)x.a);
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

	inline glm::vec4& operator>>(object o, glm::vec4& t)
	{
		GLfloat x, y, z, w;

		if(o.type != type::ARRAY) { throw type_error(); }
		if(o.via.array.size != 4) { throw type_error(); }
		o.via.array.ptr[0].convert(&x);
		o.via.array.ptr[1].convert(&y);
		o.via.array.ptr[2].convert(&z);
		o.via.array.ptr[3].convert(&w);

		t = glm::vec4(x, y, z, w);

		return t;
	}

	template <typename Stream>
	inline packer<Stream>& operator<<(packer<Stream>& o, const glm::vec4& x)
	{
		o.pack_array(4);
		o.pack((GLfloat)x.x);
		o.pack((GLfloat)x.y);
		o.pack((GLfloat)x.z);
		o.pack((GLfloat)x.w);
		return o;
	}
}
