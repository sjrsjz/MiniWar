#ifndef BUILDING_H
#define BUILDING_H
#include "Player.h"
#include <vector>
#include <string>
class Building {
	int level;
	std::string name;
public:
	Building(std::string);
	~Building();
	bool upLevel(int MaxLevel);
	std::string getName();
	int getLevel();
	bool remove();
};

#endif