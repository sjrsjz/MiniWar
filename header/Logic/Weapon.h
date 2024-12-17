#pragma once
<<<<<<< HEAD

class Weapon {

=======
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
>>>>>>> 342fef05818501dee7608a80ac7adf078302a9c5
};