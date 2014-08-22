#pragma once

#include <functional>

class scope_guard
{
public:
	typedef std::function<void()> callback_t;

private:
	callback_t m_f;

public:
	scope_guard(const callback_t& f)
	: m_f(f)
	{}

	~scope_guard()
	{
		m_f();
	}
};
