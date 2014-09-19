#pragma once

#include <memory>
#include <fstream>

#include "gl.hpp"
#include "glm_include.hpp"

class texture
{
public:
	size_t size;
	std::unique_ptr<GLubyte[]> data;

private:
	texture(size_t size)
	: size(size)
	, data(new GLubyte[size * size * 4])
	{}

public:
	static texture load_tga(std::string filename, size_t size)
	{
		texture t(size);

		std::ifstream fi(filename, std::ios::in | std::ios::binary);
		fi.seekg(18);
		if(!fi.read((char*)t.data.get(), size*size*4))
			throw std::runtime_error(std::string("Read error in texture ")+filename);

		fi.close();
		return t;
	}
};
