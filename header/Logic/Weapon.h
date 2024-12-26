#pragma once
#include <vector>
#include <tuple>


class Weapon {
	float damage;
	float damageRange;
	float attackSpeed;
	std::vector<int> cost;
	int AICost;
	std::tuple<float, float> attackRange;
	int id;

public:
	Weapon(int id);
	~Weapon();
	float getDamage(int level);
	float getDamageRange(int level);
	float getAttackSpeed(int level);
	std::tuple<float, float> getAttackRange();
	std::vector<int> getCost();
	int getAICost();
	int getId();
};
