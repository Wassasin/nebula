#pragma once

#include <vector>
#include <msgpack.hpp>

#define GLM_SWIZZLE
#include <glm/glm.hpp>

template<typename T, size_t X, size_t Y, size_t Z>
class volume
{
public:
	static constexpr size_t size = X*Y*Z;

private:
	std::vector<T> m_data;

public:
	volume()
	: m_data()
	{
		m_data.resize(size);
	}

	T& operator[](const glm::uvec3& pos)
	{
		return m_data[pos.x * Y * Z + pos.y * Z + pos.z];
	}

	const T& operator[](const glm::uvec3& pos) const
	{
		return m_data[pos.x * Y * Z + pos.y * Z + pos.z];
	}

	void operator+=(const T x)
	{
		for(size_t i = 0; i < size; ++i)
			m_data[i] += x;
	}

	void operator*=(const T x)
	{
		for(size_t i = 0; i < size; ++i)
			m_data[i] *= x;
	}

	void operator/=(const T x)
	{
		for(size_t i = 0; i < size; ++i)
			m_data[i] /= x;
	}

    MSGPACK_DEFINE(m_data)
};
