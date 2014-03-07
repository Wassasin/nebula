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
	std::pair<size_t, size_t> m_size;

	rendercontext();

public:
	glm::mat4 mvp;

	void add_cb(rcphase p, const callback_t& f);
	const std::pair<size_t, size_t>& size() const;

	rendercontext(rendercontext&) = delete;
	void run(int argc, char** argv) = delete;
};
