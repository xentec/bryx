#pragma once

#include "global.h"
#include "map.h"
#include "move.h"
#include "player.h"

///TODO: State analysis: check for moves and possible map expansion

struct Player;

struct Game
{
	Map* map;
	std::vector<Player*> players;

	struct {
		u32 players;
		u32 overrides;
		u32 bombs;
		u32 bombsStrength;
	} defaults;

	struct {
		u32 moves;
		u32 overrides;
		u32 inversions;
	} stats;

	~Game();

	void addPlayer(Player* player, const string& name = "");
	Player& nextPlayer();

	bool hasEnded();
	void run();

	Move::Error testMove(Move& move) const;
	void execute(Move& move);
	void pass();

	static Game load(std::istream& file);
private:
	Game();

	u32 currentPlayer;
	u32 moveless;
};

