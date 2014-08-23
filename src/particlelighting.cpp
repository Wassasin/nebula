#include "particlelighting.hpp"

#include "util/glm_include.hpp"
#include <glm/gtx/intersect.hpp>

#include "tri.hpp"

static inline glm::vec3 downcast(const glm::uvec3 p)
{
	return glm::vec3((GLfloat)p.x / 255, (GLfloat)p.y / 255, (GLfloat)p.z / 255);
}

static inline glm::uvec3 upcast(const glm::vec3 p)
{
	glm::vec3 tmp = glm::round(p * glm::vec3(255, 255, 255));
	return glm::clamp(glm::uvec3(tmp.x, tmp.y, tmp.z), glm::uvec3(0), glm::uvec3(255));
}

static constexpr GLfloat _A = 0.525731112119133606f;
static constexpr GLfloat _B = 0.850650808352039932f;

static const glm::uvec3 icosa_indices[20] =
{
	{0,4,1},{0,9,4},{9,5,4},{4,5,8},{4,8,1},
	{8,10,1},{8,3,10},{5,3,8},{5,2,3},{2,7,3},
	{7,10,3},{7,6,10},{7,11,6},{11,0,6},{0,1,6},
	{6,1,10},{9,0,11},{9,11,2},{9,2,5},{7,2,11}
};

static const glm::vec3 icosa_verts[12] =
{
	{_A,0.0,-_B},{-_A,0.0,-_B},{_A,0.0,_B},{-_A,0.0,_B},
	{0.0,-_B,-_A},{0.0,-_B,_A},{0.0,_B,-_A},{0.0,_B,_A},
	{-_B,-_A,0.0},{_B,-_A,0.0},{-_B,_A,0.0},{_B,_A,0.0}
};

template<size_t LAYER>
inline size_t find_tri(glm::vec3 orig, glm::vec3 dir, tri t, size_t index)
{
	glm::vec3 ab = glm::normalize(t.a + t.b / 2.0f);
	glm::vec3 ac = glm::normalize(t.a + t.c / 2.0f);
	glm::vec3 bc = glm::normalize(t.b + t.c / 2.0f);

	tri subdiv_tris[4] = {
		{t.a, ab, ac},
		{t.b, bc, ab},
		{t.c, ac, bc},
		{ab, bc, ac}
	};

	for(size_t i = 0; i < 4; i++)
	{
		const tri& t2 = subdiv_tris[i];
		glm::vec3 bary_pos;
		if(glm::intersectRayTriangle(orig, dir, t2.a, t2.b, t2.c, bary_pos))
			return find_tri<LAYER-1>(orig, dir, t2, index*4+i);
	}

	assert(false);
}

template<>
inline size_t find_tri<0>(glm::vec3, glm::vec3, tri, size_t index)
{
	return index;
}

template<size_t LAYERS>
size_t line_to_index(const glm::vec3 orig, const glm::vec3 targ)
{
	glm::vec3 dir = glm::normalize(targ - orig);

	for(size_t i = 0; i < 20; i++)
	{
		const tri t = {
			icosa_verts[icosa_indices[i][0]],
			icosa_verts[icosa_indices[i][1]],
			icosa_verts[icosa_indices[i][2]],
		};

		glm::vec3 bary_pos;
		if(glm::intersectRayTriangle(orig, dir, t.a, t.b, t.c, bary_pos))
			return find_tri<LAYERS>(orig, dir, t, i);
	}

	assert(false);
}

void particlelighting::apply_lighting(particle_nebula_t& n)
{
	static constexpr size_t LAYERS = 2;
	static const size_t tri_count = 20*std::pow(4, LAYERS);

	std::cout << "Using " << tri_count << " tris for particle lighting computation" << std::endl;

	for(star_t& s : n.stars)
	{
		// Light left per tri
		std::vector<GLfloat> tris_lighting(tri_count, 1.0f);

		// Sort particles on distance from the light source
		for(particle_t& p : n.particles)
			p.z = glm::distance(s.pos, p.pos);

		std::sort(n.particles.begin(), n.particles.end(), [&](const particle_t& a, const particle_t& b)
		{
			return a.z > b.z;
		});

		for(particle_t& p : n.particles)
		{
			size_t i = line_to_index<LAYERS>(s.pos, p.pos);

			set_rgb(p.color, upcast((downcast(s.color) * tris_lighting[i]) * downcast(p.color.rgb())));
			tris_lighting[i] *= 0.9f;
		}
	}
}
