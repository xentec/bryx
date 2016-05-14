#include "human.h"

#include "map.h"
#include "move.h"
#include "game.h"
#include "util.h"

#include "console.h"

#include <iostream>

static void error(string what)
{
	fmt::print("{}ERR: {}{}\n", console::color::GRAY, what, console::color::RESET);
}

Human::Human(Game &game, Cell::Type color):
	Player(game, color)
{}

Human::~Human()
{}

Move Human::move()
{
	Move move { *this, nullptr };

	if(possibleMoves().empty())
		return move;

	do {
		fmt::print("Place your stone (carefully) [x y OR 'pass']: ");

		try {
			string input;
			std::cin >> input;
			if(toLower(input) == "pass")
				return move;

			vec target;
			target.x = std::stoi(input);
			std::cin >> input;
			target.y = std::stoi(input);

			move.target = &game.getMap().at(target);
		} catch(std::out_of_range& ex)
		{
			error(ex.what());
			fmt::print("Your start position is outside the map!\n");
			continue;
		} catch(std::exception& ex)
		{
			error(ex.what());
			fmt::print("Something bad happened! Try again.\n");
			continue;
		}

		evaluate(move);

		move.print();

		if(move.err != Move::Error::NONE)
		{
			fmt::print("You move is invalid: {}\n", Move::err2str(move.err));
			continue;
		}

		if(move.override)
		{
			fmt::print("Your move needs an override stone");
			if(overrides == 0)
			{
				fmt::print(", but you don't have any! Try something else or pass.\n");
				continue;
			} else
			{
				fmt::print(". If you want to use one, type 'y': ");
				string input;
				std::cin >> input;
				if(toLower(input) != "y")
				{
					fmt::print("Aborted!\n");
					continue;
				}
			}
		}
		break;
	} while(true);

	if(move.target)
	{
		switch(move.target->type)
		{
		case Cell::Type::BONUS:
			fmt::print("You have hit a bonus stone!\n");
			do
			{
				fmt::print("Choose between an extra bomb (b) or override stone (o): ");

				string input;
				std::cin >> input;
				string choice = toLower(input);

				if(choice == "b")
					fmt::print("You have now {} bombs.", bombs+1);
				else if(choice == "o")
					fmt::print("You have now {} override stones.", overrides+1);
				else
				{
					fmt::print("'{}' is not a valid choice!\n", input);
					continue;
				}
				break;
			} while(true);
			break;
		case Cell::Type::CHOICE:
			fmt::print("You have hit a choice stone!\n");
			do
			{
				fmt::print("Select another players (or yours) color by entering his number (1-{}): ", game.getPlayers().size());
				u32 desired = 1;
				std::cin >> desired;

				if(desired < 1 || game.getPlayers().size() < desired)
				{
					fmt::print("Your choice '{}' is not valid player!", desired);
					continue;
				}
			} while(false);
			break;
		default:
			break;
		}
	}

	return move;
}
