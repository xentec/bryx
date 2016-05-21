#include "packet.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

using namespace packet;

template<class T> inline T& to(u8& data) { return *reinterpret_cast<T*>(&data); }
template<class T> inline T to(const u8& data) { return *reinterpret_cast<const T*>(&data); }

// Header
Header::Header(Type type):
	type(type), size(0)
{}

Header::Header(const Payload& data):
	size(0)
{
	type = static_cast<Type>(data[0]);
	size = ntohl(to<u32>(data[1]));
}

Payload Header::dump()
{
	Payload data(5+size, 0);

	data[0] = type;
	to<u32>(data[1]) = htonl(size); // set 4 fields from [1] as u32
	return data;
}


// Map
Map::Map():
	Map("")
{}

Map::Map(const string& map):
	hdr(MAP),
	map(map)
{}

Map::Map(const Payload& data):
	hdr(data),
	map(data.begin()+5, data.end())
{
	fmt::print("MAP: {}", map);
	std::fflush(stdout);
}

Payload Map::dump()
{
	hdr.size = map.size();
	Payload data = hdr.dump();

	//data.insert(data.begin()+5, map.begin(), map.end());
	std::copy(map.begin(), map.end(), data.begin()+5);
	return data;
}

// Player ID
PlayerID::PlayerID():
	PlayerID(0)
{}

PlayerID::PlayerID(u8 id):
	hdr(PLAYER_ID),
	player(id)
{
	hdr.size = 1;
}

PlayerID::PlayerID(const Payload &data):
	hdr(data),
	player(data[5])
{}

Payload PlayerID::dump()
{
	Payload data = hdr.dump();
	data[5] = player;
	return data;
}

// Move request
MoveRequest::MoveRequest():
	MoveRequest(0,0)
{}

MoveRequest::MoveRequest(u32 time, u8 depth):
	hdr(MOVE_REQUEST),
	time(time), depth(depth)
{
	hdr.size = 4+1;
}

MoveRequest::MoveRequest(const Payload &data):
	hdr(data),
//	time(ntohl(*(u32*)&data[5])),
	time(ntohl(to<u32>(data[5]))),
	depth(data[9])
{}

Payload MoveRequest::dump()
{
	Payload data = hdr.dump();

//	*(u32*) &data[5] = htonl(time); // set 4 fields from [1] as u32
	to<u32>(data[5]) = htonl(time);
	data[9] = depth;

	return data;
}

// a foreign move
Move::Move():
	Move({0,0}, 0, 0)
{}

Move::Move(const vec &pos, u8 selection, u8 player):
	hdr(MOVE),
	x(pos.x), y(pos.y), extra(selection),
	player(player)
{
	hdr.size = 2+2+1+1;
}

Move::Move(const Payload &data):
	hdr(data),
	x(ntohs(to<u16>(data[5]))), y(ntohs(to<u16>(data[7]))), extra(data[9]),
	player(data[10])
{}

Payload Move::dump()
{
	Payload data = hdr.dump();

	to<u16>(data[5]) = htons(x);
	to<u16>(data[7]) = htons(y);
	data[9] = extra;
	data[10] = player;

	return data;
}

// Join
Join::Join():
	Join(0)
{}

Join::Join(u8 group):
	hdr(JOIN),
	group(group)
{
	hdr.size = 1;
}

Join::Join(const Payload& data):
	hdr(data),
	group(data[5])
{}

Payload Join::dump()
{
	Payload data = hdr.dump();

	data[5] = group;
	return data;
}

// Move response
MoveResponse::MoveResponse():
	MoveResponse({0,0}, 0)
{}

MoveResponse::MoveResponse(const vec& pos, u8 selection):
	hdr(MOVE_RESPONSE),
	x(pos.x), y(pos.y), extra(selection)
{
	hdr.size = 2+2+1;
}

MoveResponse::MoveResponse(const Payload& data):
	hdr(data),
	x(ntohs(to<u16>(data[5]))), y(ntohs(to<u16>(data[7]))), extra(data[9])
{}

Payload packet::MoveResponse::dump()
{
	Payload data = hdr.dump();

	to<u16>(data[5]) = htons(x);
	to<u16>(data[7]) = htons(y);
	data[9] = extra;

	return data;
}

Disq::Disq():
	Disq(0)
{}

Disq::Disq(u8 player):
	hdr(DISQ),
	player(player)
{
	hdr.size = 1;
}

Disq::Disq(const Payload &data):
	hdr(data),
	player(data[5])
{}

Payload Disq::dump()
{
	Payload data = hdr.dump();

	data[5] = player;

	return data;
}

BombPhase::BombPhase():
	hdr(BOMB_PHASE)
{}

BombPhase::BombPhase(const Payload& data):
	BombPhase()
{}

Payload BombPhase::dump()
{
	return hdr.dump();
}

GameEnd::GameEnd():
	hdr(GAME_END)
{}

GameEnd::GameEnd(const Payload& data):
	GameEnd()
{}

Payload GameEnd::dump()
{
	return hdr.dump();
}
