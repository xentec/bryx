#pragma once

#include "global.h"
#include "map.h"
#include "move.h"
#include "player.h"

#include <chrono>

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
	Game(const Game& game);
	~Game();

	void load(std::istream& file);

	template<class P>
	Player& addPlayer();
	Player& nextPlayer();

	Map& getMap() const;
	std::vector<Player*>& getPlayers();

	bool hasEnded();
	void run();

	void execute(Move& move);

protected:
	Map* map;
	std::vector<Player*> players;

	u32 currentPlayer;
	u32 moveless;
};


template<class P>
Player& Game::addPlayer()
{
	if(players.size() == defaults.players)
		throw std::runtime_error("game is full");

	Player* player = new P(*this, ply2type(players.size()));
	player->bombs = defaults.bombs;
	player->overrides = defaults.overrides;
	players.push_back(player);
	return *players.back();
}
