#pragma once
#include "Player.h"
#include <vector>
#include <string>
class Building {
	int level;
	int labor;
	std::string name;
public:
	Building(std::string);
	~Building();
	bool upLevel(int MaxLevel, int labor);
	std::string getName();
	bool remove();
};
