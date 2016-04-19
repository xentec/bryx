#include "human.h"

#include "map.h"
#include "move.h"
#include "game.h"
#include "util.h"

#include <iostream>
#include <sstream>

static void error(string what)
{
	fmt::print("{}ERR: {}{}\n", color::GRAY, what, color::RESET);
}

Human::Human(const string& name):
	Player(name)
{}

Human::~Human()
{}

Move Human::move()
{
	Move move { *this, nullptr, Direction::N };

	if(possibleMoves().empty())
		return move;

	Vec2 start;
	string dirStr;

	do {
		fmt::print("Select your move (carefully) [x y direction OR 'pass']: ");

		try {
			string input;
			std::cin >> input;
			if(toLower(input) == "pass")
				return move;

			Vec2 start;
			start.x = std::stoi(input);
			std::cin >> input;
			start.y = std::stoi(input);
			std::cin >> input;

			fmt::print("VEC: {} \n", start);

			move.start = &game->map->at(start);
			move.dir = str2dir(input);
		} catch(std::out_of_range& ex)
		{
			error(ex.what());
			fmt::print("Your start position is outside the map!\n");
			continue;
		} catch(std::runtime_error& ex)
		{
			error(ex.what());
			fmt::print("Your move's direction is invalid! Choose from ");
			for(u32 i = Direction::N; i < Direction::LAST-1; i++)
				fmt::print("{}, ", dir2str(i));

			fmt::print("{}.\n", dir2str(Direction::LAST-1));
			continue;
		} catch(std::exception& ex)
		{
			error(ex.what());
			fmt::print("Something bad happened! Try again.\n");
			continue;
		}

		game->testMove(move);
		std::unordered_set<Cell*> hl {move.stones.begin(), move.stones.end()};
		hl.insert(move.start);
		hl.insert(move.end);

		switch(move.err)
		{
		case Move::Error::LINE_FULL:
			game->map->print(hl);

			fmt::print("Your move needs an override stone");
			if(overrides == 0)
			{
				fmt::print(", but you don't have any! Try something else or pass.\n");
			} else
			{
				fmt::print(". If you want to use one, type 'y': ");
				move.override = toLower(readline(std::cin)) == "y";
				goto ret;
			}
			break;
		case Move::Error::NONE:	goto ret; break;
		default:
			fmt::print("You move is invalid: {}\n", Move::err2str(move.err));
		}
	} while(true);

ret:
	return move;
}

void Human::bonus()
{
	fmt::print("You have hit a bonus stone!\n");
	do
	{
		fmt::print("Choose between an extra bomb (b) or override stone (o): ");

		string input;
		std::cin >> input;
		string choice = toLower(input);

		if(choice == "b")
		{
			bombs++;
			fmt::print("You have now {} bombs.", bombs);
		}
		else if(choice == "o")
		{
			overrides++;
			fmt::print("You have now {} override stones.", overrides);
		}	else
		{
			fmt::print("'{}' is not a valid choice!\n", input);
			continue;
		}
	} while(false);
}

Player& Human::choice()
{
	fmt::print("You have hit a choice stone!\n");
	u32 desired = 1;
	do
	{
		fmt::print("Select another players (or yours) color by entering his number (1-{}): ", game->players.size());

		std::cin >> desired;

		if(desired < 1 || game->players.size() < desired)
		{
			fmt::print("Your choice '{}' is not valid player!", desired);
			continue;
		}
	} while(false);
	return *game->players[desired-1];
}