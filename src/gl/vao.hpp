// Copyright 2013 Maurice Bos
//
// This file is part of Moggle.
//
// Moggle is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Moggle is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Moggle. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <utility>
#include <vector>

#include "gl.hpp"
#include "vbo.hpp"

class vao {

private:
	mutable GLuint id = 0;

public:
	explicit vao(bool create_now = false) {
		if (create_now) create();
	}

	~vao() { destroy(); }

	vao(vao const &) = delete;
	vao & operator = (vao const &) = delete;

	vao(vao && v) : id(v.id) { v.id = 0; }
	vao & operator = (vao && v) { std::swap(id, v.id); return *this; }

	bool created() const { return id; }
	explicit operator bool() const { return created(); }

	void create() const { if (!id) gl::generate_vertex_arrays(1, &id); }
	void destroy() { gl::delete_vertex_arrays(1, &id); id = 0; }

	void bind() const {
		create();
		gl::bind_vertex_array(id);
	}

	void attribute(
		GLuint index,
		generic_vbo const & vbo,
		size_t size,
		GLenum type,
		bool normalize_integers,
		size_t stride,
		void const * offset
	) {
		bind();
		vbo.bind(GL_ARRAY_BUFFER);
		gl::enable_vertex_attribute_array(index);
		gl::vertex_attribute_pointer(index, size, type, normalize_integers, stride, offset);
	}

	template<typename Element, typename Member>
	void attribute(GLuint, vbo<Element> &&, Member Element::*);

	template<typename Element>
	void attribute(GLuint, vbo<Element> &&);
};
