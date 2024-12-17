#pragma once
#include "Player.h"
#include <vector>
#include <string>
class Building {
	int level;
	std::vector<int> cost;

	//黄金、石油、钢材、电力、人力,巡航,中程,洲际,战力
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
};
