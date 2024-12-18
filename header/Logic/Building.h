#pragma once
#include "Player.h"
#include <vector>
#include <string>
class Building {
	int level;
	//gold,oil,elec,steel,labor,ICBM,MRBM,CM,fighter
	std::string name;
public:
	Building(std::string);
	~Building();
	bool upLevel(int MaxLevel);
	std::string getName();
	bool remove();
};
