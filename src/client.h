#pragma once

#include "global.h"
#include "socket.h"
#include "packet.h"

#include "ai.h"
#include "game.h"

struct Client
{
	Client(Game& game);

	void join(string host, u16 port = 7777);

	void play();

private:
	packet::Header peek() const;

	template<class Packet>
	Packet recv() const;

	template<class Packet>
	void send(Packet packet) const;

	Socket socket;
	Game& game;
	Player* me;
};

template<class Packet>
void Client::send(Packet packet) const
{
	socket.send(reinterpret_cast<u8*>(&packet), packet.hdr.size()+5);
}

template<class Packet>
Packet Client::recv() const
{
	packet::Header hdr;
	socket.recv(reinterpret_cast<u8*>(&hdr), 5, true);

	Packet def;
	if(def.hdr.type != hdr.type)
		throw std::runtime_error(fmt::format("wrong packet! expected: {}, got {}", def.hdr.type, hdr.type));

	socket.recv(reinterpret_cast<u8*>(&def), 5+hdr.size());

	return def;
}

