#pragma once
#include "../Timer.h"
#include "../utils/Point.h"
#include "../../header/Logic/Weapon.h"

#include "../../header/Logic/Army.h"

#include <string>
#include <cmath>
#include <vector>
#include <queue>
#include <unordered_map>

//class RegionManager {
//public:
//	static RegionManager& getInstance();
//};
class RegionManager;

class Player {
private:
	int gold{};
	int oil{};
	int electricity{};
	int labor{};
	int steel{};
	int id{};
	std::tuple<int, int> capital = std::make_tuple(-1,-1);
	std::vector<int> arm_level = {1,0,0,0};//0: army, 1: CM 2: MRBM 3: ICBM
	std::vector<int> institution_level_limit = {1,1,1,1,0};//0: powerstation, 1: steelmill, 2: oilwell, 3: civilian_factory, 4: military_factory
	RegionManager& regionmanager;
	//double calculate_distance(Point start, Point end, std::vector<std::tuple<int, int>>& path);
	//double calculate_Euclidean_distance(std::tuple<int, int> start, std::tuple<int, int> end);
public:
	Player();
	~Player();
	int get_gold();
	int get_electricity();
	int get_labor();
	int get_steel();
	int get_oil();
	int get_capital_x();
	int get_capital_y();
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

	int get_building_level_limit(std::string name);

	void move_army(Point start, Point end, int amount);
	void attack(Point start, Point end, int weapon_id);

	void build(std::string building_name, Point location);
	void upgrade_building(Point location);
	void research(int selection);
	void remove_building(Point location);

	void update(Timer timer);
};

