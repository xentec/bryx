#pragma once

#include "global.h"
#include "map.h"
#include "move.h"
#include "player.h"

struct Player;

struct Game
{
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

		struct {
			std::chrono::duration<double> moveAvg, moveMax;
		} time;
	} stats;

	Game();
	~Game();

	void load(std::istream& file);

	Player& addPlayer(Player* player);
	Player& nextPlayer();

	Map& getMap() const;
	std::vector<Player*> getPlayers() const;

	bool hasEnded();
	void run();

	void evaluate(Move& move) const;
	void execute(Move& move);

protected:
	Map* map;
	std::vector<Player*> players;

	u32 currentPlayer;
	u32 moveless;
};

