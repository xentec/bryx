#pragma once

#include "global.h"
#include "map.h"
#include "move.h"

#include "util/clock.h"

struct Player;
struct Client;

using Bomb = vec;

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
		bool gameNearEnd;
		u32 amountMoves;
	} aiData;

	enum class Phase
	{
		REVERSI, BOMB
	} phase;

	Game();
	Game(const Game& game);
	~Game();

	void load(std::istream& file);

	template<class P>
	P& addPlayer();

	Player& currPlayer() const;

	Map& getMap() const;
	std::vector<Player*>& getPlayers();
	std::vector<Player*> getPlayers() const;

	bool hasEnded();
	void run();

	void execute(Move& move);
	void undo();

	Move& getLastMove();
	usz getMoveNum() const;

	void printInfo() const;

protected:
	Player& nextPlayer();
	Player& prevPlayer();

	Map* map;
	std::vector<Player*> players;

	u32 currPly;
	u32 moveless;

	struct MoveBackup
	{
		Move move;

		Cell::Type targetType;
		std::deque<std::pair<vec, Cell::Type> > captures;
	};

	void handleSpecial(MoveBackup& mb, bool undo = false);

	std::stack<MoveBackup> moveLog;

	friend struct Client;
};


#include "player.h"

template<class P>
P& Game::addPlayer()
{
	if(players.size() == defaults.players)
		throw std::runtime_error("game is full");

	P* player = new P(*this, ply2type(players.size()));
	player->bombs = defaults.bombs;
	player->overrides = defaults.overrides;
	players.push_back(player);
	return *player;
}
