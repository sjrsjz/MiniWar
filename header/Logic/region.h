#pragma once
#include <vector>
#include "Building.h"
#include "Weapon.h"
#include "Army.h"
#include "Player.h"
#include "../utils/Config.h"
#include "../utils/Point.h"
class Region {
private:
	Building building = Building("none");
	float hp;
	float maxHp; 
	int owner;
	Point position;
	std::vector<int> weapons;
	Army army;
	/* int availableLabor; */
	/* int allLabor; */
	/* int availableElectricity; */
	/* int allElectricity; */

public:
	Region();
	Region(int x, int y);
	inline Region(Region& region) {
		// copy constructor
		this->building = region.building;
		this->hp = region.hp;
		this->maxHp = region.maxHp;
		this->owner = region.owner;
		this->position = region.position;
		this->weapons = region.weapons;
		this->army = region.army;
	}
	Region(const Region& region) {
		// copy constructor
		this->building = region.building;
		this->hp = region.hp;
		this->maxHp = region.maxHp;
		this->owner = region.owner;
		this->position = region.position;
		this->weapons = region.weapons;
		this->army = region.army;

	}
	~Region();
	bool setOwner(int owner);//in manage to judge if exists this player
	int getOwner();
	bool increaseHp(float hp);
	bool decreaseHp(float hp);
	float getHp();
	bool setBuilding(Building& building);
	Building& getBuilding();
	Point getPosition();
	bool addWeapon(int weapon, int num = 1);
	bool removeWeapon(int weapon);
	std::vector<int> getWeapons();
	bool removeArmy(int num);
	bool addArmy(int num);
	Army& getArmy();
	bool removeBuilding();
	bool setHp(float hp);
	bool setMaxHp(float hp);
	/* int getAvailableLabor(); */
	/* int getAllLabor(); */
	/* int getAvailableElectricity(); */
	/* int getAllElectricity(); */
	/* bool addAvailableLabor(int labor); */
	/* bool addAllLabor(int labor); */
	/* bool addAvailableElectricity(int electricity); */
	/* bool addAllElectricity(int electricity); */
	/* bool removeAvailableLabor(int labor); */
	/* bool removeAllLabor(int labor); */
	
	
};
