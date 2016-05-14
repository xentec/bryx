#pragma once

#include "global.h"
#include "vector.h"

#include <vector>

namespace packet
{

using Payload = std::vector<u8>;

enum Type : u8
{
	JOIN = 1,
	MAP = 2,
	PLAYER_ID = 3,
	MOVE_REQUEST = 4,
	MOVE_RESPONSE = 5,
	MOVE = 6,
	DISQ = 7,
	BOMB_PHASE = 8,
	GAME_END = 9
};


struct Header
{
	Type type;
	u32 size;

	Header(Type type);
	Header(const Payload& data);

	Payload dump();
};

// Server
//#########

struct Map
{
	Header hdr;
	string map;

	Map();
	Map(const string& map);
	Map(const Payload& data);

	Payload dump();
};

struct PlayerID
{
	Header hdr;
	u8 player;

	PlayerID();
	PlayerID(u8 player);
	PlayerID(const Payload& data);

	Payload dump();
};

struct MoveRequest
{
	Header hdr;
	u32 time;
	u8 depth;

	MoveRequest();
	MoveRequest(u32 time, u8 depth);
	MoveRequest(const Payload& data);

	Payload dump();
};

struct Move
{
	Header hdr;
	u16 x, y;
	u8 extra;
	u8 player;

	Move();
	Move(const vec& pos, u8 selection, u8 player);
	Move(const Payload& data);

	Payload dump();
};

struct Disq
{
	Header hdr;
	u8 player;

	Disq();
	Disq(u8 player);
	Disq(const Payload& data);

	Payload dump();
};


struct BombPhase
{
	Header hdr;
	
	BombPhase();
	BombPhase(const Payload& data);
		
	Payload dump();
};


struct GameEnd
{
	Header hdr;
	
	GameEnd();
	GameEnd(const Payload& data);
		
	Payload dump();
};



// Client
//#########

struct Join
{
	Header hdr;
	u8 group;

	Join();
	Join(u8 group);
	Join(const Payload& data);

	Payload dump();
};

struct MoveResponse
{
	Header hdr;
	u16 x, y;
	u8 extra;

	MoveResponse();
	MoveResponse(const vec& pos, u8 selection);
	MoveResponse(const Payload& data);

	Payload dump();
};

}
