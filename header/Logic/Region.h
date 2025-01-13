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
	Building m_building = Building(BuildingType::None);
	double m_HP;
	double m_max_HP;
	int m_owner;
	Point m_position;
	std::vector<int> m_weapons;
	Army m_army;

public:
	Region();
	Region(int x, int y);
	inline Region(Region& region) {
		// copy constructor
		this->m_building = region.m_building;
		this->m_HP = region.m_HP;
		this->m_max_HP = region.m_max_HP;
		this->m_owner = region.m_owner;
		this->m_position = region.m_position;
		this->m_weapons = region.m_weapons;
		this->m_army = region.m_army;
	}
	Region(const Region& region) {
		// copy constructor
		this->m_building = region.m_building;
		this->m_HP = region.m_HP;
		this->m_max_HP = region.m_max_HP;
		this->m_owner = region.m_owner;
		this->m_position = region.m_position;
		this->m_weapons = region.m_weapons;
		this->m_army = region.m_army;

	}
	~Region();
	bool set_owner(int owner);//in manage to judge if exists this player
	int get_owner();
	bool increase_HP(double hp);
	bool decrease_HP(double hp);
	double get_HP();
	bool set_building(Building& building);
	Building& get_building();
	Point get_center_position();
	bool add_weapon_amount(int weapon, int num = 1);
	bool remove_weapon(int weapon);
	std::vector<int> get_weapons();
	bool reduce_army_amount(int num);
	bool add_army_amount(int num);
	Army& get_army();
	bool remove_building();
	bool set_HP(double hp);
	bool set_max_HP(double hp);

	
};
