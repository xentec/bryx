#pragma once

#include "global.h"
#include "socket.h"
#include "packet.h"

#include "ai.h"
#include "game.h"

struct Client
{
	Client();

	void join(string host, u16 port = 7777);
	
	void play();

private:
	packet::Header peek() const;

	template<class Packet>
	Packet read() const;

	template<class Packet>
	void send(Packet packet) const;

	Socket socket;
	Game game;
	Player* me;
};

template<class Packet>
void Client::send(Packet packet) const
{
	socket.send(packet.dump());
}

template<class Packet>
Packet Client::read() const
{
	packet::Header hdr = peek();
	Packet def;
	if(def.hdr.type != hdr.type)
		throw std::runtime_error(fmt::format("wrong packet! expected: {}, got {}", def.hdr.type, hdr.type));

	return Packet(socket.recv(5+hdr.size));
}

