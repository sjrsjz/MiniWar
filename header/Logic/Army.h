#pragma once

class Army {
	double speed;
	int level;
	int force;
public:
	Army(int force = 0, int level = 1);
	~Army();
	double getSpeed();
	int getLevel();
	int getForce();
	bool addArmy(int num);
	bool removeArmy(int num);
	void setArmy(int num);
};
