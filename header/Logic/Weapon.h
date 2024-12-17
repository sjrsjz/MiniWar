#pragma once
#include <vector>


class Weapon {
	float damage;
	float damageRange;
	float attackSpeed;
	std::vector<int> cost;
	std::tuple<float, float> attackRange;
	int id;
	int level;

public:
	Weapon();
	~Weapon();
	bool upLevel();
	float getDamage();
	float getDamageRange();
	float getAttackSpeed();
	std::tuple<float, float> getAttackRange();
	std::vector<int> getCost();
	int getId();
	int getLevel();
};