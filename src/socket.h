#pragma once

#include "global.h"
#include <vector>

struct Socket
{
	Socket();
	~Socket();

	void connect(string host, u16 port);
	void close();

	void send(const std::vector<u8>& payload) const;
	std::vector<u8> recv(usz size, bool peek = false, bool wait = true) const;
private:
	int fd;
};
