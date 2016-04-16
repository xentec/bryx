#pragma once

#include "global.h"
#include "map.h"
#include "move.h"
#include "player.h"

///TODO: State analysis: check for moves and possible map expansion

struct Player;
struct Game
{
	Cell::Type me = Cell::Type::P1;
	std::vector<Player> players;
	u32 overrides;
	u32 bombs;
	u32 bombsStrength;
	Map* map;

	~Game();

	std::vector<Move> possibleMoves() const;
	std::vector<Move> possibleMovesOn(Cell& cell) const;
	Move::Error testMove(Move& move) const;
	void execute(const Move& move);

	static Game load(std::istream& file);
private:
	Game();
};

