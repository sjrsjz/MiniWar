#pragma once

class Army {
	double m_speed;
	int m_level;
	int m_force;
public:
	Army(int force = 0, int level = 1);
	~Army();
	double get_speed();
	int get_level();
	int get_force();
	bool add_amount(int num);
	bool reduce_amount(int num);
	void set_amount(int num);
};
