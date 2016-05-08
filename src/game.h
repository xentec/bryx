#pragma once

#include "global.h"
#include "map.h"
#include "move.h"
#include "player.h"

struct Player;

struct Game
{
	Map map;
	std::vector<Player*> players;

	struct Defaults {
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

	void addPlayer(Player* player);
	Player& nextPlayer();

	bool hasEnded();
	void run();

	Move::Error testMove(Move& move) const;
	void execute(Move& move);
	void pass();

	static Game load(std::istream& file);
private:
	Game(Map map);

	u32 currentPlayer;
	u32 moveless;
};

