#pragma once
#include "Player.h"
#include <vector>
class Building {
	int level;
	std::vector<int> cost;
	std::vector<int> production;
public:
	Building();
	~Building();
	bool upLevel(Player player);
};
