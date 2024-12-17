#include <vector>
#pragma once
#include "Building.h"
#include "Weapon.h"
#include "Player.h"
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
	bool IncreaseHp(int hp);
	bool DecreaseHp(int hp);
	int getHp();
	void setBuilding(Building building);
	Building getBuilding();
	bool addWeapon(Player player, Weapon weapon);
	bool removeWeapon(Weapon weapon);
	std::vector<Weapon> getWeapons();
	bool Attack(int x, int y, Weapon weapon);
	bool Intercept(Weapon weapon);

};
