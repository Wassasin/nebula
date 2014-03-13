#pragma once

#include <array>

// The gradients are the midpoints of the vertices of a cube.
/*static constexpr int grad3[12][3] = {
	{1,1,0}, {-1,1,0}, {1,-1,0}, {-1,-1,0},
	{1,0,1}, {-1,0,1}, {1,0,-1}, {-1,0,-1},
	{0,1,1}, {0,-1,1}, {0,1,-1}, {0,-1,-1}
};*/

static const std::array<glm::vec3, 12> grad3({{
	glm::vec3(1,1,0), glm::vec3(-1,1,0), glm::vec3(1,-1,0), glm::vec3(-1,-1,0),
	glm::vec3(1,0,1), glm::vec3(-1,0,1), glm::vec3(1,0,-1), glm::vec3(-1,0,-1),
	glm::vec3(0,1,1), glm::vec3(0,-1,1), glm::vec3(0,1,-1), glm::vec3(0,-1,-1)
}});

class simplex
{
private:
	std::vector<int> m_p;

	static int fastfloor(const float x)
	{
		return x > 0 ? (int) x : (int) x - 1;
	}

	static float dot(const glm::vec3 g, const float x, const float y, const float z)
	{
		return g[0]*x + g[1]*y + g[2]*z;
	}

public:
	simplex(unsigned int seed)
	{
		m_p.resize(256);
		std::iota(m_p.begin(), m_p.end(), 0);

		std::default_random_engine engine(seed);
		std::shuffle(m_p.begin(), m_p.end(), engine);

		// Duplicate the permutation vector
		m_p.insert(m_p.end(), m_p.begin(), m_p.end());
	}

	float noise(glm::vec3 pos) const
	{
		float n0, n1, n2, n3; // Noise contributions from the four corners

		// Skew the input space to determine which simplex cell we're in
		float F3 = 1.0/3.0;
		float s = (pos.x+pos.y+pos.z)*F3; // Very nice and simple skew factor for 3D
		int i = fastfloor(pos.x+s);
		int j = fastfloor(pos.y+s);
		int k = fastfloor(pos.z+s);

		float G3 = 1.0/6.0; // Very nice and simple unskew factor, too
		float t = (i+j+k)*G3;
		float X0 = i-t; // Unskew the cell origin back to (x,y,z) space
		float Y0 = j-t;
		float Z0 = k-t;
		float x0 = pos.x-X0; // The x,y,z distances from the cell origin
		float y0 = pos.y-Y0;
		float z0 = pos.z-Z0;

		int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
		int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords

		if(x0>=y0)
		{
			if(y0>=z0)
			{
				i1=1; j1=0; k1=0;
				i2=1; j2=1; k2=0;
			} // X Y Z order
			else if(x0>=z0)
			{
				i1=1; j1=0; k1=0;
				i2=1; j2=0; k2=1;
			} // X Z Y order
			else
			{
				i1=0; j1=0; k1=1;
				i2=1; j2=0; k2=1;
			} // Z X Y order
		}
		else
		{ // x0<y0
			if(y0<z0)
			{
				i1=0; j1=0; k1=1;
				i2=0; j2=1; k2=1;
			} // Z Y X order
			else if(x0<z0)
			{
				i1=0; j1=1; k1=0;
				i2=0; j2=1; k2=1;
			} // Y Z X order
			else
			{
				i1=0; j1=1; k1=0;
				i2=1; j2=1; k2=0;
			} // Y X Z order
		}

		// A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
		// a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
		// a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
		// c = 1/6.
		float x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
		float y1 = y0 - j1 + G3;
		float z1 = z0 - k1 + G3;
		float x2 = x0 - i2 + 2.0*G3; // Offsets for third corner in (x,y,z) coords
		float y2 = y0 - j2 + 2.0*G3;
		float z2 = z0 - k2 + 2.0*G3;
		float x3 = x0 - 1.0 + 3.0*G3; // Offsets for last corner in (x,y,z) coords
		float y3 = y0 - 1.0 + 3.0*G3;
		float z3 = z0 - 1.0 + 3.0*G3;

		// Work out the hashed gradient indices of the four simplex corners
		int ii = i & 255;
		int jj = j & 255;
		int kk = k & 255;
		int gi0 = m_p[ii+m_p[jj+m_p[kk]]] % 12;
		int gi1 = m_p[ii+i1+m_p[jj+j1+m_p[kk+k1]]] % 12;
		int gi2 = m_p[ii+i2+m_p[jj+j2+m_p[kk+k2]]] % 12;
		int gi3 = m_p[ii+1+m_p[jj+1+m_p[kk+1]]] % 12;

		// Calculate the contribution from the four corners
		float t0 = 0.6 - x0*x0 - y0*y0 - z0*z0;
		if(t0<0)
			n0 = 0.0;
		else
		{
			t0 *= t0;
			n0 = t0 * t0 * dot(grad3[gi0], x0, y0, z0);
		}

		float t1 = 0.6 - x1*x1 - y1*y1 - z1*z1;
		if(t1<0)
			n1 = 0.0;
		else
		{
			t1 *= t1;
			n1 = t1 * t1 * dot(grad3[gi1], x1, y1, z1);
		}

		float t2 = 0.6 - x2*x2 - y2*y2 - z2*z2;
		if(t2<0)
			n2 = 0.0;
		else
		{
			t2 *= t2;
			n2 = t2 * t2 * dot(grad3[gi2], x2, y2, z2);
		}

		float t3 = 0.6 - x3*x3 - y3*y3 - z3*z3;
		if(t3<0)
			n3 = 0.0;
		else
		{
			t3 *= t3;
			n3 = t3 * t3 * dot(grad3[gi3], x3, y3, z3);
		}

		// Add contributions from each corner to get the final noise value.
		// The result is scaled to stay just inside [-1,1]
		return 32.0*(n0 + n1 + n2 + n3);
	}

	float octave_noise(
		const float octaves,
		const float persistence,
		const float scale,
		const glm::vec3 pos
	) const
	{
		float total = 0;
		float frequency = scale;
		float amplitude = 1;

		float maxAmplitude = 0;

		for(size_t i = 0; i < octaves; ++i)
		{
			total += noise( pos * frequency ) * amplitude;

			frequency *= 2;
			maxAmplitude += amplitude;
			amplitude *= persistence;
		}

		return total / maxAmplitude;
	}
};
