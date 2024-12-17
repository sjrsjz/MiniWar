#pragma once
#include "Player.h"
#include <vector>
#include <string>
class Building {
	int level;
	std::vector<int> cost;

	//gold,oil,elec,steel,labor,ICBM,MRBM,CM,fighter
	std::vector<int> production;
	std::string name;
public:
	Building(std::string);
	~Building();
	bool upLevel(Player& player);
	std::vector<int> product();
	std::vector<int> getCost();
	std::string getName();
	bool remove(Player& player);
	void update(Player& player);
};
