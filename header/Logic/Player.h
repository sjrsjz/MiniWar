#pragma once
#include "../Timer.h"
#include "../utils/Point.h"
#include "../../header/Logic/Weapon.h"

#include "../../header/Logic/Army.h"
#include "../../header/Logic/Building.h"
#include "../../header/utils/GlobalTimer.h"
#include "../../header/utils/Operation.h"

#include <string>
#include <cmath>
#include <vector>
#include <queue>
#include <unordered_map>


class RegionManager;
class Region;

class Player {
private:
	struct {
		double gold{};
		double oil{};
		double electricity{};
		double steel{};
		double labor{};
	}m_resources; // 累计资源
	struct {
		double gold{};
		double oil{};
		double electricity{};
		double steel{};
		double labor{};
	}m_steady_cost_resources; // 固定的资源消耗

	bool power_off = false;
	int id{};
	bool have_research_institution = false;
	std::tuple<int, int> capital = std::make_tuple(-1,-1);
	const std::vector<int> max_army_level = { 3, 3, 3, 3 };
	std::vector<int> army_level = { 1,0,0,0 };//0: army, 1: CM 2: MRBM 3: ICBM
	const std::vector<int> max_institution_level = { 3,3,3,3,3 };
	std::vector<int> institution_level_limit = { 1,1,1,1,1 };//0: powerstation, 1: refinery, 2: steelfactory, 3: civilian_factory, 4: military_factory
	RegionManager& regionmanager;

public:
	Player();
	~Player();
	std::vector<double> get_resources() {
		std::vector<double> resources = { m_resources.gold, m_resources.oil, m_resources.steel, m_resources.electricity, m_resources.labor };
		return resources;
	}
	std::vector<double> get_steady_cost_resources() {
		std::vector<double> resources = { m_steady_cost_resources.gold, m_steady_cost_resources.oil, m_steady_cost_resources.steel, m_steady_cost_resources.electricity, m_steady_cost_resources.labor };
		return resources;
	}

	std::vector<double> get_remain_resources() {
		std::vector<double> remain_resources = {
			std::fmax(0, m_resources.gold - m_steady_cost_resources.gold),
			std::fmax(0, m_resources.oil - m_steady_cost_resources.oil),
			std::fmax(0, m_resources.steel - m_steady_cost_resources.steel),
			std::fmax(0, m_resources.electricity - m_steady_cost_resources.electricity),
			std::fmax(0, m_resources.labor - m_steady_cost_resources.labor)
		};
		return remain_resources;
	}

	int get_capital_x();
	int get_capital_y();
	void upgrade_building_level_limit(BuildingType building_type);
	void upgrade_army_level_limit();
	void upgrade_weapon_level_limit(int weapon_type);
	int get_building_level_limit(BuildingType type);
	int get_army_level(int id) {
		try {
			return army_level[id];
		}
		catch (std::exception e) {
			throw e;
		}
	}
	bool get_have_research_institution() {
		return have_research_institution;
	}
	bool is_alive();

	//Interaction functions below
	void move_army(Operation operation, int amount);
	void attack(Operation operation);

	void build(Operation operation);
	void upgrade_building(Operation operation);
	void set_research(Operation operation);
	void research(Operation operation);
	void remove_building(Operation operation);
	void product_weapon(int weapon_type, Region& region);
	void product(Operation operation);
	void rangeAttack(Operation operation);
	void create();

	//update function
	void update(GlobalTimer& timer);
};

