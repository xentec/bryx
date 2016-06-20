#include "human.h"

#include "map.h"
#include "move.h"
#include "game.h"

#include "util/console.h"

#include <iostream>

static void error(string what)
{
	println("{}ERR: {}{}\n", console::color::GRAY, what, console::color::RESET);
}

Human::Human(Game &game, Cell::Type color):
	Player(game, color)
{}

Human::Human(const Human &other):
	Player(other)
{}

Human::~Human()
{}

Player *Human::clone() const
{
	return new Human(*this);
}

Move Human::move(std::deque<Move>&, u32, u32)
{
	Move move { *this, nullptr };

	do {
		print("Place your stone (carefully) [x y]: ");
		move.clear();

		try {
			string input;
			std::cin >> input;
			vec target;
			target.x = std::stoi(input);
			std::cin >> input;
			target.y = std::stoi(input);

			move.target = &game.getMap().at(target);
		} catch(std::out_of_range& ex)
		{
			error(ex.what());
			println("Your start position is outside the map!");
			continue;
		} catch(std::exception& ex)
		{
			error(ex.what());
			println("Something bad happened! Try again.");
			continue;
		}

		evaluate(move);

		move.print();

		if(move.err != Move::Error::NONE)
		{
			println("You move is invalid: {}", Move::err2str(move.err));
			continue;
		}

		if(move.override)
		{
			println("Your move needs an override stone");
			if(overrides == 0)
			{
				println(", but you don't have any! Try something else.");
				continue;
			} else
			{
				print(". If you want to use one, type 'y': ");
				string input;
				std::cin >> input;
				if(toLower(input) != "y")
				{
					println("Aborted!");
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
			println("You have hit a bonus stone!");
			do
			{
				print("Choose between an extra bomb (b) or override stone (o): ");

				string input;
				std::cin >> input;
				string choice = toLower(input);

				if(choice == "b")
					println("You have now {} bombs.", bombs+1);
				else if(choice == "o")
					println("You have now {} override stones.", overrides+1);
				else
				{
					println("'{}' is not a valid choice!", input);
					continue;
				}
				break;
			} while(true);
			break;
		case Cell::Type::CHOICE:
			println("You have hit a choice stone!");
			do
			{
				print("Select another players (or yours) color by entering his number (1-{}): ", game.getPlayers().size());
				u32 desired = 1;
				std::cin >> desired;

				if(desired < 1 || game.getPlayers().size() < desired)
				{
					println("Your choice '{}' is not valid player!", desired);
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
