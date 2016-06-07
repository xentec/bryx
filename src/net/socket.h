#pragma once

#include "global.h"
#include <vector>

struct Socket
{
	Socket();
	~Socket();

	void connect(string host, u16 port);
	void close();

	usz send(const u8* buffer, usz size) const;
	usz recv(u8* buffer, usz size, bool peek = false, bool wait = true) const;
private:
	int fd;
};
