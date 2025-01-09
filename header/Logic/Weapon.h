#pragma once
#include <vector>
#include <tuple>


class Weapon {
	double damage;
	double damageRange;
	double attackSpeed;
	std::vector<int> cost;
	int AICost;
	std::tuple<double, double> attackRange;
	int id;

public:
	Weapon(int id);
	~Weapon();
	double getDamage(int level);
	double getDamageRange(int level);
	double getAttackSpeed(int level);
	std::tuple<double, double> getAttackRange();
	std::vector<int> getCost();
	int getAICost();
	int getId();
};
