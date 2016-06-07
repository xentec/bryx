#pragma once

#include <functional>
#include <stack>

struct Scope
{
	using Callback = std::function<void()>;

	Scope() = default;
	Scope(Callback cb);
	~Scope();

	Scope& operator += (Callback cb);
	Callback operator --();
	Scope operator -- (int);
private:
	std::stack<Callback> cbs;
};
