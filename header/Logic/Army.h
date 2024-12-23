#pragma once

class Army {
	float speed;
	int level;
	int force;
public:
	Army(int force = 0, int level = 1);
	~Army();
	float getSpeed();
	int getLevel();
	int getForce();
	bool addArmy(int num);
	bool removeArmy(int num);
};
