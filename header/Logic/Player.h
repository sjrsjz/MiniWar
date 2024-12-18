#pragma once
#include "../Timer.h"
#include "../utils/Point.h"
#include "Army.h"
#include <cmath>
#include <vector>
#include <queue>
#include <unordered_map>

class RegionManager;

class Player {
private:
	int gold{};
	int oil{};
	int electricity{};
	int labor{};
	int steel{};
	std::vector<int> arm_level;//0: army, 1: CM 2: MRBM 3: ICBM
	std::vector<int> institution_level_limit;//0: powerstation, 1: steelmill, 2: oilwell, 3: civilian_factory, 4: military_factory, 5: research_center
	RegionManager& regionmanager;
	float calculate_distance(Point start, Point end);
public:
	int id;
	Player();
	~Player();
	int get_gold();
	int get_electricity();
	int get_labor();
	int get_steel();
	int get_oil();
	void gold_cost(int cost);
	void oil_cost(int cost);
	void electricity_cost(int cost);
	void labor_cost(int cost);
	void steel_cost(int cost);
	void add_gold(int amount);
	void add_oil(int amount);
	void add_electricity(int amount);
	void add_labor(int amount);
	void add_steel(int amount);
	void move_army(Point start, Point end, int amount);
	void update(Timer timer);
};

