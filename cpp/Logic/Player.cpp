#include "../../header/Logic/Player.h"
#include "../../header/Logic/RegionManager.h"
#include "../../header/debug.h"
#include "../../header/utils/Config.h"
#include <random>

Player:: Player(): regionmanager(RegionManager::getInstance()){
    id = 0;
}

Player:: ~Player() {
}

int Player:: get_gold(){
    return gold;
}

int Player:: get_electricity(){
    return electricity;
}

int Player:: get_labor_limit(){
    return labor_limit;
}

int Player::get_ocupied_labor() {
	return ocupied_labor;
}

int Player:: get_steel(){
    return steel;
}

int Player:: get_oil(){
    return oil;
}

int Player::get_capital_x() {
	return std::floor(std::get<0>(capital));
}

int Player::get_capital_y() {
	return std::floor(std::get<1>(capital));
}

void Player:: gold_cost(int cost){
    if (gold < cost){
		DEBUG::DebugOutput("Player::gold_cost() throws");
		throw std::invalid_argument(u8"金钱不足");
    }
    gold -= cost;
}

void Player:: oil_cost(int cost){
    if (oil < cost){
		DEBUG::DebugOutput("Player::oil_cost() throws");
		throw std::invalid_argument(u8"石油不足");
    }
    oil -= cost;
}

void Player:: electricity_cost(int cost){
    if (electricity < cost){
		DEBUG::DebugOutput("Player::electricity_cost() throws");
		throw std::invalid_argument(u8"电力不足");
    }
    electricity -= cost;
}

void Player:: labor_cost(int cost){
    if (labor_limit - ocupied_labor < cost){
		DEBUG::DebugOutput("Player::labor_cost() throws");
		throw std::invalid_argument(u8"劳动力不足");
    }
    ocupied_labor += cost;
}

void Player:: steel_cost(int cost){
    if (steel < cost){
		DEBUG::DebugOutput("Player::steel_cost() throws");
		throw std::invalid_argument(u8"钢铁不足");
    }
    steel -= cost;
}

void Player:: add_gold(int amount){
    gold += amount;
}

void Player:: add_oil(int amount){
    oil += amount;
}

void Player:: add_electricity(int amount){
    electricity += amount;
}

void Player:: add_labor(int amount){
    labor_limit += amount; 
}// not sure

void Player:: add_steel(int amount){
    steel += amount;
}

bool Player::is_alive() {
	for (int i{}; i < regionmanager.get_map_width(); i++) {
		for (int j{}; j < regionmanager.get_map_height(); j++) {
			if (regionmanager.get_region(i, j).getOwner() == id) {
				return true;
			}
		}
	}
	return false;
}

int Player::get_building_level_limit(std::string name) {
    if (name == "PowerStation") return institution_level_limit[0];
	if (name == "Refinery") return institution_level_limit[1];
    if (name == "SteelFactory") return institution_level_limit[2];
	if (name == "CivilFactory") return institution_level_limit[3];
	if (name == "MilitaryFactory") return institution_level_limit[4];
	return -1;
}

void Player:: move_army(Operation operation, int amount){
 //   std::vector<std::tuple<int, int>> path;
 //   double distance = calculate_distance(start, end, path);
 //   if (distance == -1) {
 //       throw "Can't find a path";
 //   }

 //   int start_x = std::floor(start.getX());
 //   int start_y = std::floor(start.getY());
 //   int end_x = std::floor(end.getX());
 //   int end_y = std::floor(end.getY());

 //   Region& start_region = regionmanager.get_region(start_x, start_y);
 //   Region& end_region = regionmanager.get_region(end_x, end_y);

 //   double time = distance / start_region.getArmy().getSpeed();

 //   start_region.removeArmy(amount);
 //   //create a new army to move, when time is up, this army shall be destroyed, and the end region add this army's force
	//regionmanager.move_army(amount, time, path);
	Point start = operation.getStart();
	Point end = operation.getEnd();

    int start_x = std::floor(start.getX());
    int start_y = std::floor(start.getY());
    int end_x = std::floor(end.getX());
    int end_y = std::floor(end.getY());

	Region& start_region = regionmanager.get_region(start_x, start_y);
	Region& end_region = regionmanager.get_region(end_x, end_y);

	if (start_region.getOwner() != id) {
		throw std::invalid_argument(u8"区块未被占有");
	}
	if (start_region.getArmy().getForce() < amount || amount == 0) {
		throw std::invalid_argument(u8"军队数量不足");
	}

	regionmanager.move_army(start, end, amount, arm_level[0]);
}

void Player::attack(Operation operation) {
	Point start = operation.getStart();
	Point end = operation.getEnd();
	double distance_origin = sqrt(pow(start.getX() - end.getX(), 2) + pow(start.getY() - end.getY(), 2));
	double distance = sqrt(pow(start.getX() - end.getX(), 2) + pow(start.getY() - end.getY(), 2)) / fmax(regionmanager.get_map_width(), regionmanager.get_map_height());

    int start_x = std::floor(start.getX());
    int start_y = std::floor(start.getY());

    Region& start_region = regionmanager.get_region(start_x, start_y);

	int weapon_id = 0;

	switch (operation.getOp())
	{
	case Operator::Weapon0Attack:
		weapon_id = 0;
		break;
	case Operator::Weapon1Attack:
		weapon_id = 1;
		break;
	case Operator::Weapon2Attack:
		weapon_id = 2;
		break;
	}

	Weapon weapon = regionmanager.get_weapon(weapon_id);

	std::tuple<float, float> AttackRange= weapon.getAttackRange();
	if (start_region.getWeapons()[weapon_id] <= 0) {
		throw std::invalid_argument(u8"武器数量不足");
	}

	if (distance > std::get<1>(AttackRange) || distance < std::get<0>(AttackRange)) {
		throw std::invalid_argument(u8"超出攻击范围");
	}

    double time = distance_origin / weapon.getAttackSpeed(arm_level[id+1]);

	start_region.removeWeapon(weapon_id);

	regionmanager.attack_region_missle(weapon_id, arm_level[weapon_id + 1], start, end, time);
} //should detect if the weapon can reach the target

void Player::build(Operation operation) {

	Point location = operation.getCur();
	std::string building_name = "";

	switch (operation.getOp())
	{
	case Operator::SetPowerStation:
		building_name = "PowerStation";
		break;
	case Operator::SetRefinery:
		building_name = "Refinery";
		break;
	case Operator::SetSteelFactory:
		building_name = "SteelFactory";
		break;
	case Operator::SetCivilFactory:
		building_name = "CivilFactory";
		break;
	case Operator::SetMilitaryFactory:
		building_name = "MilitaryFactory";
		break;
	}

	Region& region = regionmanager.get_region(std::floor(location.getX()), std::floor(location.getY()));
	if (region.getOwner() != id) {
		throw std::invalid_argument(u8"区块未被占有");
	}
    if (region.getBuilding().getName() != "none") {
		throw std::invalid_argument(u8"已放置建筑");
	}
    //wait for configure.h complete
	Config& configer = Config::getInstance();
	json BuildCost = configer.getConfig({ "Building",building_name, "BuildCost" });
	std::vector<int> Level1Cost = BuildCost.template get<std::vector<int>>();
	if (gold < Level1Cost[0] || oil < Level1Cost[1] || steel < Level1Cost[2] || electricity < Level1Cost[3] || labor_limit - ocupied_labor < Level1Cost[4]) {
		throw std::invalid_argument(u8"资源不足");
	}
	gold -= Level1Cost[0];
	oil -= Level1Cost[1];
	steel -= Level1Cost[2];
	electricity -= Level1Cost[3];
	ocupied_labor += Level1Cost[4];
	Building building(building_name);
	region.setBuilding(building);
}

void Player::upgrade_building(Operation operation) {
	Point location = operation.getCur();
	Region& region = regionmanager.get_region(std::floor(location.getX()), std::floor(location.getY()));
    if (region.getBuilding().getName() == "none") {
        throw std::invalid_argument(u8"当前区块没有建筑");
    }
	if (region.getOwner() != id) {
		throw std::invalid_argument(u8"区块未被占有");
	}

	//wait for configure.h complete
	//make sure cost is enough, then uplevel
    if (region.getBuilding().getLevel() == institution_level_limit[get_building_level_limit(region.getBuilding().getName())]) {
        throw std::invalid_argument(u8"已升级到最高级");
    }

	Config& configer = Config::getInstance();
    switch (region.getBuilding().getLevel())
    {
    case 1:
		gold -= configer.getConfig({ "Building",region.getBuilding().getName(), "UpLevelCost1", "Gold" }).get<int>();
		oil -= configer.getConfig({ "Building",region.getBuilding().getName(), "UpLevelCost1", "Oil" }).get<int>();
		steel -= configer.getConfig({ "Building",region.getBuilding().getName(), "UpLevelCost1", "Steel" }).get<int>();
		electricity -= configer.getConfig({ "Building",region.getBuilding().getName(), "UpLevelCost1", "Electricity" }).get<int>();
		ocupied_labor += configer.getConfig({ "Building",region.getBuilding().getName(), "UpLevelCost1", "Labor" }).get<int>();
		break;  
	case 2:
		gold -= configer.getConfig({ "Building",region.getBuilding().getName(), "UpLevelCost2", "Gold" }).get<int>();
		oil -= configer.getConfig({ "Building",region.getBuilding().getName(), "UpLevelCost2", "Oil" }).get<int>();
		steel -= configer.getConfig({ "Building",region.getBuilding().getName(), "UpLevelCost2", "Steel" }).get<int>();
		electricity -= configer.getConfig({ "Building",region.getBuilding().getName(), "UpLevelCost2", "Electricity" }).get<int>();
		ocupied_labor += configer.getConfig({ "Building",region.getBuilding().getName(), "UpLevelCost2", "Labor" }).get<int>();
		break;
    }

    region.getBuilding().upLevel(institution_level_limit[get_building_level_limit(region.getBuilding().getName())]);
}

void Player::remove_building(Operation operation) {
	Point location = operation.getCur();
	Region& region = regionmanager.get_region(std::floor(location.getX()), std::floor(location.getY()));
    if (region.getOwner() != id) {
		throw std::invalid_argument(u8"区块未被占有");
	}
    if (region.getBuilding().getName() == "none") {
		throw std::invalid_argument(u8"区块没有建筑");
	}
	//return the cost of the building

	Building& building = region.getBuilding();
	Config& configer = Config::getInstance();
    json BuildCost = configer.getConfig({ "Building",building.getName(), "BuildCost" });
    json UpLevelCost1 = configer.getConfig({ "Building",building.getName(), "UpLevelCost1" });
    json UpLevelCost2 = configer.getConfig({ "Building",building.getName(), "UpLevelCost2" });
    std::vector<int> Level1Cost = BuildCost.template get<std::vector<int>>();
    std::vector<int> Level2Cost = UpLevelCost1.template get<std::vector<int>>();
    std::vector<int> Level3Cost = UpLevelCost2.template get<std::vector<int>>();
    switch (building.getLevel())
    {
    case 1:
		gold += std::floor(0.6 * Level1Cost[0]);
		oil += std::floor(0.6 * Level1Cost[1]);
		steel += std::floor(0.6 * Level1Cost[2]);
		electricity += std::floor(0.6 * Level1Cost[3]);
		ocupied_labor -= Level1Cost[4];
		break;
    case 2:
        gold += std::floor(0.6 * (Level1Cost[0] + Level2Cost[0]));
        oil += std::floor(0.6 * (Level1Cost[1] + Level2Cost[1]));
        steel += std::floor(0.6 * (Level1Cost[2] + Level2Cost[2]));
        electricity += std::floor(0.6 * (Level1Cost[3] + Level2Cost[3]));
        ocupied_labor -= Level1Cost[4] + Level2Cost[4];
        break;
    case 3:
		gold += std::floor(0.6 * (Level1Cost[0] + Level2Cost[0] + Level3Cost[0]));
		oil += std::floor(0.6 * (Level1Cost[1] + Level2Cost[1] + Level3Cost[1]));
		steel += std::floor(0.6 * (Level1Cost[2] + Level2Cost[2] + Level3Cost[2]));
		electricity += std::floor(0.6 * (Level1Cost[3] + Level2Cost[3] + Level3Cost[3]));
		ocupied_labor -= Level1Cost[4] + Level2Cost[4] + Level3Cost[4];
		break;
    }
	region.removeBuilding();
}

void Player::set_research(Operation operation) {
	Config& configer = Config::getInstance();
	int cost = configer.getConfig({ "ResearchInstitution","BuildCost" }).get<int>();
	if (gold < cost) {
		throw std::invalid_argument(u8"金钱不足");
	}
	gold -= cost;
	have_research_institution = true;
}

void Player::research(Operation operation) {
	if (!have_research_institution) {
		throw std::invalid_argument(u8"未解锁研究所");
	}
	//wait for configure.h complete
	//make sure cost is enough, then research
    Config& configer = Config::getInstance();
	std::vector<int> Uplevelcost_PowerStation = configer.getConfig({ "ResearchInstitution","OUpLevelCost","PowerStation" }).template get<std::vector<int>>();
	std::vector<int> Uplevelcost_Refinery = configer.getConfig({ "ResearchInstitution","OUpLevelCost","Refinery" }).template get<std::vector<int>>();
	std::vector<int> Uplevelcost_SteelFactory = configer.getConfig({ "ResearchInstitution","OUpLevelCost","SteelFactory" }).template get<std::vector<int>>();
	std::vector<int> Uplevelcost_CivilFactory = configer.getConfig({ "ResearchInstitution","OUpLevelCost","CivilFactory" }).template get<std::vector<int>>();
	std::vector<int> Uplevelcost_MilitaryFactory = configer.getConfig({ "ResearchInstitution","OUpLevelCost","MilitaryFactory" }).template get<std::vector<int>>();

	std::vector<int> Uplevelcost_Army = configer.getConfig({ "ResearchInstitution","OUpLevelCost","Army" }).template get<std::vector<int>>();
	std::vector<int> Uplevelcost_CM = configer.getConfig({ "ResearchInstitution","OUpLevelCost","0" }).template get<std::vector<int>>();
	std::vector<int> Uplevelcost_MRBM = configer.getConfig({ "ResearchInstitution","OUpLevelCost","1" }).template get<std::vector<int>>();
	std::vector<int> Uplevelcost_ICBM = configer.getConfig({ "ResearchInstitution","OUpLevelCost","2" }).template get<std::vector<int>>();

	switch (operation.getOp()) {
	case Operator::PowerStationUpLevel:
		if (institution_level_limit[0] == 3) {
			throw std::invalid_argument(u8"已升级到最高等级");
		}
		else {
			if (institution_level_limit[0] == 1) {
				if (gold < Uplevelcost_PowerStation[0]) {
					throw std::invalid_argument(u8"金钱不足");
				}
				else {
					gold -= Uplevelcost_PowerStation[0];
					institution_level_limit[0] = 2;
				}
			}
			else if (institution_level_limit[0] == 2) {
				if (gold < Uplevelcost_PowerStation[1]) {
					throw std::invalid_argument(u8"金钱不足");
				}
				else {
					gold -= Uplevelcost_PowerStation[1];
					institution_level_limit[0] = 3;
				}
			}
		}
		break;
	case Operator::RefineryUpLevel:
		if (institution_level_limit[1] == 3) {
			throw std::invalid_argument(u8"已升级到最高等级");
		}
		else {
			if (institution_level_limit[1] == 1) {
				if (gold < Uplevelcost_Refinery[0]) {
					throw std::invalid_argument(u8"金钱不足");
				}
				else {
					gold -= Uplevelcost_Refinery[0];
					institution_level_limit[1] = 2;
				}
			}
			else if (institution_level_limit[1] == 2) {
				if (gold < Uplevelcost_Refinery[1]) {
					throw std::invalid_argument(u8"金钱不足");
				}
				else {
					gold -= Uplevelcost_Refinery[1];
					institution_level_limit[1] = 3;
				}
			}
		}
		break;
	case Operator::SteelFactoryUpLevel:
		if (institution_level_limit[2] == 3) {
			throw std::invalid_argument(u8"已升级到最高等级");
		}
		else {
			if (institution_level_limit[2] == 1) {
				if (gold < Uplevelcost_SteelFactory[0]) {
					throw std::invalid_argument(u8"金钱不足");
				}
				else {
					gold -= Uplevelcost_SteelFactory[0];
					institution_level_limit[2] = 2;
				}
			}
			else if (institution_level_limit[2] == 2) {
				if (gold < Uplevelcost_SteelFactory[1]) {
					throw std::invalid_argument(u8"金钱不足");
				}
				else {
					gold -= Uplevelcost_SteelFactory[1];
					institution_level_limit[2] = 3;
				}
			}
		}
		break;
	case Operator::CivilFactoryUpLevel:
		if (institution_level_limit[3] == 3) {
			throw std::invalid_argument(u8"已升级到最高等级");
		}
		else {
			if (institution_level_limit[3] == 1) {
				if (gold < Uplevelcost_CivilFactory[0]) {
					throw std::invalid_argument(u8"金钱不足");
				}
				else {
					gold -= Uplevelcost_CivilFactory[0];
					institution_level_limit[3] = 2;
				}
			}
			else if (institution_level_limit[3] == 2) {
				if (gold < Uplevelcost_CivilFactory[1]) {
					throw std::invalid_argument(u8"金钱不足");
				}
				else {
					gold -= Uplevelcost_CivilFactory[1];
					institution_level_limit[3] = 3;
				}
			}
		}
		break;
	case Operator::MilitaryFactoryUpLevel:
		if (institution_level_limit[4] == 3) {
			throw std::invalid_argument(u8"已升级到最高等级");
		}
		else {
			if (institution_level_limit[4] == 1) {
				if (gold < Uplevelcost_MilitaryFactory[0]) {
					throw std::invalid_argument(u8"金钱不足");
				}
				else {
					gold -= Uplevelcost_MilitaryFactory[0];
					institution_level_limit[4] = 2;
				}
			}
			else if (institution_level_limit[4] == 2) {
				if (gold < Uplevelcost_MilitaryFactory[1]) {
					throw std::invalid_argument(u8"金钱不足");
				}
				else {
					gold -= Uplevelcost_MilitaryFactory[1];
					institution_level_limit[4] = 3;
				}
			}
		}
		break;
	case Operator::ArmyUpLevel:
		if (arm_level[0] == 3) {
			throw std::invalid_argument(u8"已升级到最高等级");
		}
		else {
			if (arm_level[0] == 1) {
				if (gold < Uplevelcost_Army[0]) {
					throw std::invalid_argument(u8"金钱不足");
				}
				else {
					gold -= Uplevelcost_Army[0];
					arm_level[0] = 2;
				}
			}
			else if (arm_level[0] == 2) {
				if (gold < Uplevelcost_Army[1]) {
					throw std::invalid_argument(u8"金钱不足");
				}
				else {
					gold -= Uplevelcost_Army[1];
					arm_level[0] = 3;
				}
			}
		}
		break;
	case Operator::Weapon0UpLevel:
		if (arm_level[1] == 3) {
			throw std::invalid_argument(u8"已升级到最高等级");
		}
		else {
			if (arm_level[1] == 0) {
				if (gold < Uplevelcost_CM[0]) {
					throw std::invalid_argument(u8"金钱不足");
				}
				else {
					gold -= Uplevelcost_CM[0];
					arm_level[1] = 1;
				}
			}
			else if (arm_level[1] == 1) {
				if (gold < Uplevelcost_CM[1]) {
					throw std::invalid_argument(u8"金钱不足");
				}
				else {
					gold -= Uplevelcost_CM[1];
					arm_level[1] = 2;
				}
			}
			else if (arm_level[1] == 2) {
				if (gold < Uplevelcost_CM[2]) {
					throw std::invalid_argument(u8"金钱不足");
				}
				else {
					gold -= Uplevelcost_CM[2];
					arm_level[1] = 3;
				}
			}	
		}
		break;
	case Operator::Weapon1UpLevel:
		if (arm_level[2] == 3) {
			throw std::invalid_argument(u8"已升级到最高等级");
		}
		else {
			if (arm_level[2] == 0) {
				if (gold < Uplevelcost_MRBM[0]) {
					throw std::invalid_argument(u8"金钱不足");
				}
				else {
					gold -= Uplevelcost_MRBM[0];
					arm_level[2] = 1;
				}
			}
			else if (arm_level[2] == 1) {
				if (gold < Uplevelcost_MRBM[1]) {
					throw std::invalid_argument(u8"金钱不足");
				}
				else {
					gold -= Uplevelcost_MRBM[1];
					arm_level[2] = 2;
				}
			}
			else if (arm_level[2] == 2) {
				if (gold < Uplevelcost_MRBM[2]) {
					throw std::invalid_argument(u8"金钱不足");
				}
				else {
					gold -= Uplevelcost_MRBM[2];
					arm_level[2] = 3;
				}
			}
		}
		break;
	case Operator::Weapon2UpLevel:
		if (arm_level[3] == 3) {
			throw std::invalid_argument(u8"已升级到最高等级");
		}
		else {
			if (arm_level[3] == 0) {
				if (gold < Uplevelcost_ICBM[0]) {
					throw std::invalid_argument(u8"金钱不足");
				}
				else {
					gold -= Uplevelcost_ICBM[0];
					arm_level[3] = 1;
				}
			}
			else if (arm_level[3] == 1) {
				if (gold < Uplevelcost_ICBM[1]) {
					throw std::invalid_argument(u8"金钱不足");
				}
				else {
					gold -= Uplevelcost_ICBM[1];
					arm_level[3] = 2;
				}
			}
			else if (arm_level[3] == 2) {
				if (gold < Uplevelcost_ICBM[2]) {
					throw std::invalid_argument(u8"金钱不足");
				}
				else {
					gold -= Uplevelcost_ICBM[2];
					arm_level[3] = 3;
				}
			}
		}
		break;
	}
}

void Player::product(Operation operation) {
	Point start = operation.getStart();
	Point end = operation.getEnd();
	Region& start_region = regionmanager.get_region(std::floor(start.getX()), std::floor(start.getY()));
	Region& end_region = regionmanager.get_region(std::floor(end.getX()), std::floor(end.getY()));

	if (start_region.getOwner() != id || end_region.getOwner() != id) {
		throw std::invalid_argument(u8"区块未被占有");
	}
	if (start_region.getBuilding().getName() != "MilitaryFactory") {
		throw std::invalid_argument(u8"没有军事工厂");
	}
	Config& configer = Config::getInstance();
	int cost{};
	std::vector<int> cost0;
	switch (operation.getOp())
	{
	case Operator::ProductArmy:
		cost = configer.getConfig({ "Army","cost" }).get<int>() * operation.getSize();
		if (gold < cost) {
			throw std::invalid_argument(u8"金钱不足");
		}
		gold -= cost;
		end_region.addArmy(operation.getSize());
		break;
	case Operator::ProductWeapon0:
		if (arm_level[1] == 0) {
			throw std::invalid_argument(u8"未解锁短程核导弹");
		}
		cost0 = configer.getConfig({ "Weapon", "0", "cost" }).template get<std::vector<int>>();
		if (gold < cost0[0] || oil < cost0[1] || electricity < cost0[2] || steel < cost0[3] || labor_limit - ocupied_labor < cost0[4]) {
			throw std::invalid_argument(u8"资源不足");
		}
		gold -= cost0[0];
		oil -= cost0[1];
		electricity -= cost0[2];
		steel -= cost0[3];	
		end_region.addWeapon(0);
		break;
	case Operator::ProductWeapon1:
		if (arm_level[2] == 0) {
			throw std::invalid_argument(u8"未解锁中程核导弹");
		}
		cost0 = configer.getConfig({ "Weapon", "1", "cost" }).template get<std::vector<int>>();
		if (gold < cost0[0] || oil < cost0[1] || electricity < cost0[2] || steel < cost0[3] || labor_limit - ocupied_labor < cost0[4]) {
			throw std::invalid_argument(u8"资源不足");
		}
		gold -= cost0[0];
		oil -= cost0[1];
		electricity -= cost0[2];
		steel -= cost0[3];
		end_region.addWeapon(1);
		break;
	case Operator::ProductWeapon2:
		if (arm_level[3] == 0) {
			throw std::invalid_argument(u8"未解锁远程核导弹");
		}
		cost0 = configer.getConfig({ "Weapon", "2", "cost" }).template get<std::vector<int>>();
		if (gold < cost0[0] || oil < cost0[1] || electricity < cost0[2] || steel < cost0[3] || labor_limit - ocupied_labor < cost0[4]) {
			throw std::invalid_argument(u8"资源不足");
		}
		gold -= cost0[0];
		oil -= cost0[1];
		electricity -= cost0[2];
		steel -= cost0[3];
		end_region.addWeapon(2);
		break;
	}
}

void Player:: update(GlobalTimer& timer){
	std::vector<double> delta_resource = { 0,0,0,0,0 };
	double delta_t = timer.get_dt();
	regionmanager.calculate_delta_resources(delta_resource, delta_t, id);
	gold += delta_resource[0];
	oil += delta_resource[1];
	steel += delta_resource[2];
	electricity += delta_resource[3];
	labor_limit = delta_resource[4];
}

void Player::rangeAttack(Operation operation) {
	Point cur = operation.getCur();
	float radius = operation.getRadius();
	int num = radius * radius / 4;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0, radius);
	bool flag = false;
	int cnt = 0;
	std::vector<std::pair<Point, std::vector<int>>> regionWeapons;
	for (int i = 0; i < regionmanager.get_map_width(); i++) {
		for (int j = 0; j < regionmanager.get_map_height(); j++) {
			Region& region = regionmanager.get_region(i, j);
			if (region.getOwner() == id) {
				regionWeapons.push_back(std::make_pair(region.getPosition(), region.getWeapons()));
			}
		}
	}
	while (num) {
		float x = dis(gen);
		float y = dis(gen);
		Point target(x, y);
		int mapSize = regionmanager.get_map_width();
		for (auto regionWeapon : regionWeapons) {
			Point regionPos = regionWeapon.first;
			std::vector<int>& weapons = regionWeapon.second;
			float distance = regionPos.distance(target);
			if (weapons[0] > 0) {
				float speed = regionmanager.get_weapon(0).getAttackSpeed(arm_level[1]);
				/*float damage = regionmanager.get_weapon(0).getDamage(arm_level[1]);*/
				double time = distance / speed;
				if (distance <= 0.25 * mapSize) {
					regionmanager.attack_region_missle(0, arm_level[1], regionPos, target, time);
					num--;
					flag = true;
				}
			} else if (weapons[1] > 0) {
				float speed = regionmanager.get_weapon(1).getAttackSpeed(arm_level[2]);
				/*float damage = regionmanager.get_weapon(1).getDamage(arm_level[2]);*/
				double time = distance / speed;
				if (distance <= 0.5 * mapSize) {
					regionmanager.attack_region_missle(1, arm_level[2], regionPos, target, time);
					num--;
					flag = true;
				}
			} else if (weapons[2] > 0) {
				float speed = regionmanager.get_weapon(2).getAttackSpeed(arm_level[3]);
				/*float damage = regionmanager.get_weapon(2).getDamage(arm_level[3]);*/
				double time = distance / speed;
				if (distance <= 0.75 * mapSize && distance >= 0.2 * mapSize) {
					regionmanager.attack_region_missle(2, arm_level[3], regionPos, target, time);
					num--;
					flag = true;
				}
			} 
		}
		if (!flag) {
			cnt++;
			if (cnt > 5) {
				break;
			}
		}
	}

}

void Player::create() {
	Config config = Config::getInstance();
	std::tuple<float, float> originSize = config.getConfig({"Region", "OriginSize"}).template get<std::tuple<float, float>>();


	int mapWidth = regionmanager.get_map_width();
	int mapHeight = regionmanager.get_map_height();
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> disX(3, mapWidth - 3);
	std::uniform_int_distribution<int> disY(3, mapHeight - 3);
	std::uniform_int_distribution<int> disSize(std::get<0>(originSize), std::get<1>(originSize));
	int size = disSize(gen);
	int x = disX(gen);
	int y = disY(gen);
	capital = std::make_tuple(x, y);
	float Hp = config.getConfig({"Region", "CapitalHp"}).get<float>();
	int Force = config.getConfig({"Region", "CapitalArmy"}).get<int>();
	regionmanager.get_region(x, y).setHp(Hp);
	regionmanager.get_region(x, y).getArmy().addArmy(Force);
	regionmanager.get_region(x, y).setOwner(0);
	for (int i = -3; i <= 3; i++) {
		for (int j = -3; j <= 3; j++) {
			if (i * i + j * j > size * size) {
				continue;
			}
			regionmanager.get_region(x + i, y + j).setOwner(0);
		}
	}

	//gold = 10000000;
	//oil = 10000000;
	//electricity = 10000000;
	//labor_limit = 1000000;
	//ocupied_labor = 0;
	//steel = 10000000;

	json playerSource = config.getConfig({ "PlayerOrigionSource" });
	gold = playerSource["gold"].get<int>();
	oil = playerSource["oil"].get<int>();
	electricity = playerSource["electricity"].get<int>();
	labor_limit = playerSource["laborLimit"].get<int>();
	ocupied_labor = playerSource["ocupiedLabor"].get<int>();
	steel = playerSource["steel"].get<int>();
	arm_level = { 1, 0, 0, 0 };

	institution_level_limit = { 1, 1, 1, 1, 1 };
	have_research_institution = false;



	return;
}

