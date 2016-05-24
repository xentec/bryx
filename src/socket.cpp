#include "socket.h"

#include "util.h"

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
		fmt::print("PORT: {}\n", ntohs(sin.sin_port));
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

void Socket::send(const std::vector<u8>& payload) const
{
#if DEBUG > 4
	fmt::print("SENDING: [{}] ", payload.size());
	std::fflush(stdout);

	fmt::print("{:02x} ", payload[0]);
	fmt::print("{:08x} ", ntohl(*(u32*) &payload[1]));

	for(auto iter = payload.cbegin()+5; iter != payload.cend(); iter++)
		fmt::print("{:02x} ", *iter);

	fmt::print("\n");
	std::fflush(stdout);
#endif

	isz len = ::send(fd, reinterpret_cast<const char*>(payload.data()), payload.size(), 0);
	if(len < 0)
		throw std::runtime_error(strerror(errno));
}

std::vector<u8> Socket::recv(usz size, bool peek, bool wait) const
{
	std::vector<u8> buffer(size);

	isz len = ::recv(fd, reinterpret_cast<char*>(&buffer[0]), buffer.size(), peek*MSG_PEEK | MSG_WAITALL);
	if(len < 0)
		throw std::runtime_error(strerror(errno));

#if DEBUG > 4
	fmt::print("RECEIVING: [{}] ", buffer.size());
	std::fflush(stdout);

	if(len > 0)
	{
		fmt::print("{:02x} ", buffer[0]);
		if(buffer.size() > 1)
		{
			fmt::print("{:08x} ", ntohl(*(u32*) &buffer[1]));
			if(buffer.size() > 5)
			{
				for(auto iter = buffer.begin()+5; iter != buffer.end(); iter++)
					fmt::print("{:02x} ", *iter);
			}
		}
	}

	fmt::print("\n");
	std::fflush(stdout);
#endif

	return buffer;
}
