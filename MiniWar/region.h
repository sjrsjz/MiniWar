#include <vector>
#pragma once
#include "Building.h"
#include "Weapon.h"
class region {
private:
	Building building;
	int hp;
	int owner;
	std::vector<Weapon> weapons;

public:
	region();
	~region();
	void setOwner(int owner);
	int getOwner();
	void setHp(int hp);
	int getHp();
	void setBuilding(Building building);
	Building getBuilding();
	void addWeapon(Weapon weapon);
	void removeWeapon(Weapon weapon);
	std::vector<Weapon> getWeapons();


};
