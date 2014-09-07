#include "particlelighting.hpp"

#include <numeric>

#include "gl/gl.hpp"
#include "gl/glm_include.hpp"
#include "gl/glm_opts.hpp"
#include <glm/gtx/intersect.hpp>

#include "tri.hpp"

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
inline size_t find_tri(glm::vec3 orig, glm::vec3 dir, tri t, size_t index, GLfloat error)
{
	glm::vec3 ab = glm::normalize((t.a + t.b) / 2.0f);
	glm::vec3 ac = glm::normalize((t.a + t.c) / 2.0f);
	glm::vec3 bc = glm::normalize((t.b + t.c) / 2.0f);

	tri subdiv_tris[4] = {
		{t.a, ab, ac},
		{t.b, bc, ab},
		{t.c, ac, bc},
		{ab, bc, ac}
	};

	error *= 3.0f;
	const GLfloat factor = 1.0f + error;
	for(size_t i = 0; i < 4; i++)
	{
		const tri& t2 = subdiv_tris[i] * factor;
		glm::vec3 bary_pos;
		if(glm::intersectRayTriangle(orig, dir, t2.a, t2.c, t2.b, bary_pos))
			return find_tri<LAYER-1>(orig, dir, t2, index*4+i, error);
	}

	assert(false);
}

template<>
inline size_t find_tri<0>(glm::vec3, glm::vec3, tri, size_t index, GLfloat)
{
	return index;
}

template<size_t LAYERS>
size_t line_to_index(const glm::vec3 orig, const glm::vec3 targ)
{
	glm::vec3 dir = glm::normalize(targ - orig);
	glm::vec3 zero(0.0, 0.0, 0.0);

	for(size_t i = 0; i < 20; i++)
	{
		const tri t = {
			icosa_verts[icosa_indices[i][0]],
			icosa_verts[icosa_indices[i][1]],
			icosa_verts[icosa_indices[i][2]],
		};

		glm::vec3 bary_pos;
		if(glm::intersectRayTriangle(zero, dir, t.a, t.c, t.b, bary_pos))
			return find_tri<LAYERS>(zero, dir, t, i, std::numeric_limits<GLfloat>::epsilon());
	}

	assert(false);
}

template<size_t LAYER>
inline void draw_tris(tri t)
{
	glm::vec3 ab = glm::normalize((t.a + t.b) / 2.0f);
	glm::vec3 ac = glm::normalize((t.a + t.c) / 2.0f);
	glm::vec3 bc = glm::normalize((t.b + t.c) / 2.0f);

	tri subdiv_tris[4] = {
		{t.a, ab, ac},
		{t.b, bc, ab},
		{t.c, ac, bc},
		{ab, bc, ac}
	};

	for(size_t i = 0; i < 4; i++)
		draw_tris<LAYER-1>(subdiv_tris[i]);
}

template<>
inline void draw_tris<0>(tri t)
{
	glColor3f(t.a.x, t.b.y, t.c.z);
	glVertex3f(t.a.x, t.a.y, t.a.z);
	glVertex3f(t.b.x, t.b.y, t.b.z);
	glVertex3f(t.c.x, t.c.y, t.c.z);
}

void particlelighting::apply_lighting(particle_nebula_t& n)
{
	static constexpr size_t LAYERS = 7;
	static const size_t tri_count = 20*std::pow(4, LAYERS);
	static const GLfloat shadowing_factor = std::pow(std::pow(0.9f, 1.0f/40.0f), std::pow(2.0f, (GLfloat)LAYERS));

	std::cout << "Using " << tri_count << " tris for particle lighting computation on " << n.particles.size() << " particles (SF " << shadowing_factor << ")" << std::endl;

	std::vector<size_t> tmp_index(n.particles.size());
	std::iota(tmp_index.begin(), tmp_index.end(), 0);

	std::vector<glm::vec3> light(n.particles.size());
	for(const star_t& s : n.stars)
	{
		// Light left per tri
		std::vector<GLfloat> tris_lighting(tri_count, 1.0f);

		// Sort particles on distance from the light source
		for(particle_t& p : n.particles)
			p.z = glm::distance(s.pos, p.pos);

		std::sort(tmp_index.begin(), tmp_index.end(), [&](const size_t a, const size_t b)
		{
			return n.particles[a].z < n.particles[b].z;
		});

		for(size_t i : tmp_index)
		{
			GLfloat dist = glm::abs(glm::length(s.pos - n.particles[i].pos));
			size_t j = line_to_index<LAYERS>(s.pos, n.particles[i].pos);

			GLfloat power = 1.0f / std::pow(dist+1.0f, 2.0f);
			//GLfloat power = 1.0f;
			light[i] += downcast(s.color) * tris_lighting[j] * power;
			tris_lighting[j] *= shadowing_factor;
		}
	}

	for(size_t i = 0; i < n.particles.size(); i++)
		set_rgb(n.particles[i].color, upcast(downcast(n.particles[i].color.rgb()) * light[i]));
}

void particlelighting::draw_debug()
{
	glBegin(GL_TRIANGLES);
	for(size_t i = 0; i < 20; i++)
	{
		const tri t = {
			icosa_verts[icosa_indices[i][0]],
			icosa_verts[icosa_indices[i][1]],
			icosa_verts[icosa_indices[i][2]],
		};

		draw_tris<5>(t);
	}
	glEnd();
}
