#pragma once

#include "global.h"
#include "map.h"
#include "move.h"
#include "player.h"

#include "util/clock.h"

struct Player;
struct Client;

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
			Duration moveAvg, moveMax;
		} time;
	} stats;

    struct {
        u32 bombValue;
        u32 expectedOverriteValue;
		Duration evalTime;
    } aiData;

	Game();
	Game(const Game& game);
	~Game();

	void load(std::istream& file);

	template<class P>
	Player& addPlayer();

	Player& currPlayer() const;
	Player& nextPlayer();
	Player& prevPlayer();

	Map& getMap() const;
	std::vector<Player*>& getPlayers();
	std::vector<Player*> getPlayers() const;

	bool hasEnded();
	void run();

	void execute(Move& move, bool backup = false);
	void undo(Move& move);

protected:
	Map* map;
	std::vector<Player*> players;

	u32 currPly;
	u32 moveless;

	friend struct Client;
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
