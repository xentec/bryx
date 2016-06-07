#pragma once

#include "global.h"
#include "vector.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif


#include <vector>

#pragma pack(push)
#pragma pack(push,1)

namespace packet
{
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
	u32 size_net;

	Header();
	Header(Type type, u32 size = 0);

	inline u32 size() const { return ntohl(size_net); }
};

// Server
//#########

struct Map
{
	Header hdr;
	const char* map_net;

	Map();
	Map(const string& map);

	inline string map() const { return string(map_net, hdr.size()); }
};

struct PlayerID
{
	Header hdr;
	u8 player;

	PlayerID();
	PlayerID(u8 player);
};

struct MoveRequest
{
	Header hdr;
	u32 time_net;
	u8 depth;

	MoveRequest();
	MoveRequest(u32 time, u8 depth);

	inline u32 time() const { return ntohl(time_net); }
};

struct Move
{
	Header hdr;
	u16 x, y;
	u8 extra;
	u8 player;

	Move();
	Move(const vec& pos, u8 selection, u8 player);

	inline vec pos() const { return vec{ ntohs(x), ntohs(y) }; }
};

struct Disq
{
	Header hdr;
	u8 player;

	Disq();
	Disq(u8 player);
};


struct BombPhase
{
	Header hdr;

	BombPhase();
};


struct GameEnd
{
	Header hdr;

	GameEnd();
};



// Client
//#########

struct Join
{
	Header hdr;
	u8 group;

	Join();
	Join(u8 group);
};

struct MoveResponse
{
	Header hdr;
	u16 x, y;
	u8 extra;

	MoveResponse();
	MoveResponse(const vec& pos, u8 selection);

	inline vec pos() const { return vec{ ntohs(x), ntohs(y) }; }
};

}

#pragma pack(pop)
