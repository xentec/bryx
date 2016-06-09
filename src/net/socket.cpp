#include "socket.h"

#include "util/console.h"
#include "util/scope.h"

#include <cstring> // memset

#include <fmt/format.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#endif

Socket::Socket():
	fd(-1)
{}

Socket::~Socket()
{
	close();
}

void Socket::connect(string host, u16 port)
{
	i32 err;
	addrinfo hints, *res = nullptr;

	// first, load up address structs with getaddrinfo():

	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

#ifdef _WIN32
	WSADATA wsaData;
	err = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (err != NO_ERROR)
		throw std::runtime_error("wsa init failed");
#endif

	// Try both IPv4 and v6 first and if it fails, try with IPv4 only
	do
	{
		err = getaddrinfo(host.c_str(), fmt::FormatInt(port).c_str(), &hints, &res);
		Scope cs([=](){ if(res) freeaddrinfo(res); });

		if(err)
			throw std::runtime_error(fmt::format("failed to resolve address: {}", gai_strerror(err)));

		fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if(fd == -1)
			throw std::runtime_error(fmt::format("failed to create socket: {}", strerror(errno)));

		err = ::connect(fd, res->ai_addr, res->ai_addrlen);
		if(err)
		{
			if(hints.ai_family != AF_INET)
			{
				hints.ai_family = AF_INET;
				continue;
			}
			throw std::runtime_error(strerror(errno));
		}

		break;
	} while(true);

#if DEBUG > 3
	sockaddr_in sin;
	socklen_t len = sizeof(sin);
	err = getsockname(fd, (sockaddr *)&sin, &len);
	if(err != -1)
		println("PORT: {}", ntohs(sin.sin_port));
#endif
}

void Socket::close()
{
#ifdef _WIN32
	closesocket(fd);
#else
	::close(fd);
#endif
}

usz Socket::send(const u8* buffer, usz size) const
{
#if DEBUG > 4
	println("SENDING: [{}] ", size);
	std::fflush(stdout);

	print("{:02x} ", buffer[0]);
	print("{:08x} ", ntohl(*(u32*) &buffer[1]));

	for(usz i = 5; i < size; ++i)
		print("{:02x} ", buffer[i]);

	println();
	std::fflush(stdout);
#endif

	isz len = ::send(fd, reinterpret_cast<const char*>(buffer), size, 0);
	if(len < 0)
		throw std::runtime_error(strerror(errno));

	return len;
}

usz Socket::recv(u8* buffer, usz size, bool peek, bool wait) const
{
	isz len = ::recv(fd, reinterpret_cast<char*>(buffer), size, peek*MSG_PEEK | wait*MSG_WAITALL);
	if(len < 0)
		throw std::runtime_error(strerror(errno));

#if DEBUG > 4
	println("RECEIVING: [{}] ", size);
	std::fflush(stdout);

	if(len > 0)
	{
		print("{:02x} ", buffer[0]);
		if(len > 1)
		{
			print("{:08x} ", ntohl(*(u32*) &buffer[1]));
			if(len > 5)
			{
				for(usz i = 5; i < size; ++i)
					print("{:02x} ", buffer[i]);
			}
		}
	}

	println();
	std::fflush(stdout);
#endif


	return len;
}
