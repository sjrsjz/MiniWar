<<<<<<< HEAD
#pragma once
#include "Player.h"
#include <vector>
#include <string>
class Building {
	int level;
	std::vector<int> cost;

	//person, 
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
=======
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
};
>>>>>>> 5d7c84a7f2fab7bbd1cf56dfc619eca428cf0272
