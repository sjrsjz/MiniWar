#pragma once
#include <vector>
#include <tuple>


class Weapon {
	float damage;
	float damageRange;
	float attackSpeed;
	std::vector<int> cost;
	std::tuple<float, float> attackRange;
	int id;

public:
	Weapon(int id);
	~Weapon();
	float getDamage(int level);
	float getDamageRange(int level);
	float getAttackSpeed(int level);
	std::tuple<float, float> getAttackRange(int level);
	std::vector<int> getCost();
	int getId();
};
