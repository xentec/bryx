#include "packet.h"

using namespace packet;

template<class T> inline T& to(u8& data) { return *reinterpret_cast<T*>(&data); }
template<class T> inline T to(const u8& data) { return *reinterpret_cast<const T*>(&data); }

Header::Header()
{}

Header::Header(Type type, u32 size):
	type(type), size_net(htonl(size))
{}

// Map
Map::Map():
	Map("")
{}

Map::Map(const string& map):
	hdr{ MAP, static_cast<u32>(map.size()) },
	map_net(map.c_str())
{}

// Player ID
PlayerID::PlayerID():
	PlayerID(0)
{}

PlayerID::PlayerID(u8 id):
	hdr{ PLAYER_ID, 1 },
	player(id)
{}

// Move request
MoveRequest::MoveRequest():
	MoveRequest(0,0)
{}

MoveRequest::MoveRequest(u32 time, u8 depth):
	hdr{ MOVE_REQUEST, 4+1 },
	time_net(htonl(time)), depth(depth)
{}

// a foreign move
Move::Move():
	Move({0,0}, 0, 0)
{}

Move::Move(const vec &pos, u8 selection, u8 player):
	hdr{ MOVE, 2+2+1+1 },
	x(htons(pos.x)), y(htons(pos.y)),
	extra(selection), player(player)
{}

// Join
Join::Join():
	Join(0)
{}

Join::Join(u8 group):
	hdr{ JOIN, 1 },
	group(group)
{}

// Move response
MoveResponse::MoveResponse():
	MoveResponse({0,0}, 0)
{}

MoveResponse::MoveResponse(const vec& pos, u8 selection):
	hdr{MOVE_RESPONSE, 2+2+1 },
	x(htons(pos.x)), y(htons(pos.y)),
	extra(selection)
{}

Disq::Disq():
	Disq(0)
{}

Disq::Disq(u8 player):
	hdr{ DISQ, 1 },
	player(player)
{}

BombPhase::BombPhase():
	hdr{ BOMB_PHASE, 0 }
{}

GameEnd::GameEnd():
	hdr{ GAME_END, 0 }
{}
