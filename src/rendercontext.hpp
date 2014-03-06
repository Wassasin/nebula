#pragma once

#include <list>
#include <map>
#include <functional>

#include <glm/glm.hpp>

enum class rcphase {
	init,
	update,
	draw,
	cleanup
};

class rendercontext
{
public:
	typedef std::function<void(rendercontext& s)> callback_t;

protected:
	std::map<rcphase, std::list<callback_t>> m_cbs;

	rendercontext();

public:
	glm::mat4 projectionmat;
	glm::mat4 viewmat;

	void add_cb(rcphase p, const callback_t& f);

	void run() = delete;
};
