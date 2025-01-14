#pragma once
#include <vector>
#include <tuple>


class Weapon {
	double m_damage;
	double m_damage_range;
	double m_attack_speed;
	std::vector<int> m_cost;
	int m_AI_cost;
	std::tuple<double, double> m_attack_range;
	int m_id;

public:
	Weapon(int id);
	~Weapon();
	double get_damage(int level);
	double get_damage_range(int level);
	double get_attack_speed(int level);
	std::tuple<double, double> get_attack_range();
	std::vector<int> get_cost();
	int get_AI_cost();
	int get_id();
};
