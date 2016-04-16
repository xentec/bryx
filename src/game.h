#pragma once

#include "global.h"
#include "map.h"
#include "move.h"
#include "player.h"

///TODO: State analysis: check for moves and possible map expansion

struct Player;

struct Game
{
	std::vector<Player> players;
	u32 overrides;
	u32 bombs;
	u32 bombsStrength;
	Map* map;

	~Game();

	Player& nextPlayer();
	bool hasEnded();

	Move::Error testMove(Move& move) const;
	void execute(Move& move);
	void pass();

	static Game load(std::istream& file);
private:
	Game();

	u32 currentPlayer;
	u32 moveless;
};

