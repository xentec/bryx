#include "scope.h"

Scope::Scope(Scope::Callback cb)
{
	cbs.push(cb);
}

Scope::~Scope()
{
	while(!cbs.empty())
	{
		// call all callbacks starting with the last
		cbs.top()();
		cbs.pop();
	}
}

Scope& Scope::operator +=(Scope::Callback cb)
{
	cbs.push(cb);
	return *this;
}

Scope::Callback Scope::operator --()
{
	Callback cb = cbs.top();
	cbs.pop();
	return cb;
}

Scope Scope::operator --(int)
{
	Scope prev = *this;
	cbs.pop();
	return prev;
}
