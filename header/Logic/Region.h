#include <vector>
#pragma once
#include "Building.h"
#include "Weapon.h"
#include "Player.h"
#include "../utils/Point.h"
class region {
private:
	Building building;
	float hp;
	int owner;
	Point position;
	std::pair<float, float> position;
	std::vector<Weapon> weapons;

public:
	region();
	~region();
	void setOwner(int owner);
	int getOwner();
	bool increaseHp(float hp);
	bool decreaseHp(float hp);
	float getHp();
	void setBuilding(Building& building);
	Building& getBuilding();
	Point getPosition();
	bool addWeapon(Weapon& weapon);
	bool removeWeapon(Weapon& weapon);
	std::vector<Weapon> getWeapons();
	bool Attack(float x, float y, Weapon& weapon);
	bool Intercept(Weapon& weapon);

};
