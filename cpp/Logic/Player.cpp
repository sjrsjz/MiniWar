#include "../../header/Logic/Player.h"
#include "../../header/Logic/RegionManager.h"
#include "../../header/debug.h"
#include "../../header/utils/Config.h"
#include "../../header/Logic/Resource.h"
#include <random>

Player:: Player(): m_region_manager(RegionManager::instance_of()){
    m_id = 0;
}

Player:: ~Player() {
}

int Player::get_capital_x() {
	return std::floor(std::get<0>(m_capital_idx));
}

int Player::get_capital_y() {
	return std::floor(std::get<1>(m_capital_idx));
}

bool Player::is_alive() {
	for (int i{}; i < m_region_manager.map_width(); i++) {
		for (int j{}; j < m_region_manager.map_height(); j++) {
			if (m_region_manager.region(i, j).get_owner() == m_id) {
				return true;
			}
		}
	}
	return false;
}

int Player::get_building_level_limit(BuildingType type) {
    if (type == BuildingType::PowerStation) return m_institution_level_limit[BuildingType::PowerStation];
	if (type == BuildingType::Refinery) return m_institution_level_limit[BuildingType::Refinery];
    if (type == BuildingType::SteelFactory) return m_institution_level_limit[BuildingType::SteelFactory];
	if (type == BuildingType::CivilFactory) return m_institution_level_limit[BuildingType::CivilFactory];
	if (type == BuildingType::MilitaryFactory) return m_institution_level_limit[BuildingType::MilitaryFactory];
	return -1;
}

void Player:: move_army(Operation operation, int amount){

	Point start = operation.getStart();
	Point end = operation.getEnd();

    int start_x = std::floor(start.x);
    int start_y = std::floor(start.y);
    int end_x = std::floor(end.x);
    int end_y = std::floor(end.y);

	Region& start_region = m_region_manager.region(start_x, start_y);
	Region& end_region = m_region_manager.region(end_x, end_y);

	if (start_region.get_owner() != m_id) {
		throw std::invalid_argument(u8"区块未被占有");
	}
	if (start_region.get_army().get_force() < amount || amount == 0) {
		throw std::invalid_argument(u8"军队数量不足");
	}

	m_region_manager.move_army(start, end, amount, m_army_level[0]);
}

void Player::attack(Operation operation) {
	Point start = operation.getStart();
	Point end = operation.getEnd();
	double distance_origin = start.distance(end);
	double distance = distance_origin / fmax(m_region_manager.map_width(), m_region_manager.map_height());

    int start_x = std::floor(start.x);
    int start_y = std::floor(start.y);

    Region& start_region = m_region_manager.region(start_x, start_y);

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

	Weapon weapon = m_region_manager.get_weapon(weapon_id);

	std::tuple<double, double> AttackRange = weapon.get_attack_range();
	if (start_region.get_weapons()[weapon_id] <= 0) {
		throw std::invalid_argument(u8"武器数量不足");
	}

	if (distance > std::get<1>(AttackRange) || distance < std::get<0>(AttackRange)) {
		throw std::invalid_argument(u8"超出攻击范围");
	}

    double time = distance_origin / weapon.get_attack_speed(m_army_level[m_id+1]);

	start_region.remove_weapon(weapon_id);

	m_region_manager.attack_region_missle(weapon_id, m_army_level[weapon_id + 1], start, end, time);
} //should detect if the weapon can reach the target

void Player::build(Operation operation) {

	Point location = operation.getCur();
	BuildingType building_type = BuildingType::None;

	switch (operation.getOp())
	{
	case Operator::SetPowerStation:
		building_type = BuildingType::PowerStation;
		break;
	case Operator::SetRefinery:
		building_type = BuildingType::Refinery;
		break;
	case Operator::SetSteelFactory:
		building_type = BuildingType::SteelFactory;
		break;
	case Operator::SetCivilFactory:
		building_type = BuildingType::CivilFactory;
		break;
	case Operator::SetMilitaryFactory:
		building_type = BuildingType::MilitaryFactory;
		break;
	}

	Region& region = m_region_manager.region(std::floor(location.x), std::floor(location.y));
	if (region.get_owner() != m_id) {
		throw std::invalid_argument(u8"区块未被占有");
	}
    if (region.get_building().get_type() != BuildingType::None) {
		throw std::invalid_argument(u8"已放置建筑");
	}
	std::vector<double> build_cost = Config::instance_of().get_building_setting(BuildingTypeToString(building_type)).BuildCost[0];
	std::vector<double> steady_cost = Config::instance_of().get_building_setting(BuildingTypeToString(building_type)).SteadyCost[0];
	if (build_cost[ResourceType::GOLD] + steady_cost[ResourceType::GOLD] > m_resources.gold - m_steady_cost_resources.gold) {
		throw std::invalid_argument(u8"金钱不足");
	}
	if (build_cost[ResourceType::OIL] + steady_cost[ResourceType::OIL] > m_resources.oil - m_steady_cost_resources.oil) {
		throw std::invalid_argument(u8"石油不足");
	}
	if (build_cost[ResourceType::ELECTRICITY] + steady_cost[ResourceType::ELECTRICITY] > m_resources.electricity - m_steady_cost_resources.electricity) {
		throw std::invalid_argument(u8"电力不足");
	}
	if (build_cost[ResourceType::STEEL] + steady_cost[ResourceType::STEEL] > m_resources.steel - m_steady_cost_resources.steel) {
		throw std::invalid_argument(u8"钢铁不足");
	}
	if (build_cost[ResourceType::LABOR] + steady_cost[ResourceType::LABOR] > m_resources.labor - m_steady_cost_resources.labor) {
		throw std::invalid_argument(u8"劳动力不足");
	}
	m_resources.gold -= build_cost[ResourceType::GOLD];
	m_resources.oil -= build_cost[ResourceType::OIL];
	m_resources.steel -= build_cost[ResourceType::STEEL];
	m_resources.electricity -= build_cost[ResourceType::ELECTRICITY];
	Building building(building_type);
	region.set_building(building);
}

void Player::upgrade_building(Operation operation) {
	Point location = operation.getCur();
	Region& region = m_region_manager.region(std::floor(location.x), std::floor(location.y));
    if (region.get_building().get_type() == BuildingType::None) {
        throw std::invalid_argument(u8"当前区块没有建筑");
    }
	if (region.get_owner() != m_id) {
		throw std::invalid_argument(u8"区块未被占有");
	}

    if (region.get_building().get_level() == m_institution_level_limit[get_building_level_limit(region.get_building().get_type())]) {
        throw std::invalid_argument(u8"已升级到最高级");
    }

	Config& configer = Config::instance_of();

	std::string building_name = BuildingTypeToString(region.get_building().get_type());
	int level = region.get_building().get_level();
	std::vector<double> build_cost = configer.get_building_setting(building_name).BuildCost[level];
	std::vector<double> steady_cost = configer.get_building_setting(building_name).SteadyCost[level];

	if (build_cost[ResourceType::GOLD] + steady_cost[ResourceType::GOLD] > m_resources.gold - m_steady_cost_resources.gold) {
		throw std::invalid_argument(u8"金钱不足");
	}
	if (build_cost[ResourceType::OIL] + steady_cost[ResourceType::OIL] > m_resources.oil - m_steady_cost_resources.oil) {
		throw std::invalid_argument(u8"石油不足");
	}
	if (build_cost[ResourceType::ELECTRICITY] + steady_cost[ResourceType::ELECTRICITY] > m_resources.electricity - m_steady_cost_resources.electricity) {
		throw std::invalid_argument(u8"电力不足");
	}
	if (build_cost[ResourceType::STEEL] + steady_cost[ResourceType::STEEL] > m_resources.steel - m_steady_cost_resources.steel) {
		throw std::invalid_argument(u8"钢铁不足");
	}
	if (build_cost[ResourceType::LABOR] + steady_cost[ResourceType::LABOR] > m_resources.labor - m_steady_cost_resources.labor) {
		throw std::invalid_argument(u8"劳动力不足");
	}


	m_resources.gold -= configer.get_building_setting(building_name).BuildCost[level][ResourceType::GOLD];
	m_resources.oil -= configer.get_building_setting(building_name).BuildCost[level][ResourceType::OIL];
	m_resources.steel -= configer.get_building_setting(building_name).BuildCost[level][ResourceType::STEEL];
	m_resources.electricity -= configer.get_building_setting(building_name).BuildCost[level][ResourceType::ELECTRICITY];
	m_resources.labor -= configer.get_building_setting(building_name).BuildCost[level][ResourceType::LABOR];


	region.get_building().up_level(m_institution_level_limit[region.get_building().get_type()]);
}

void Player::remove_building(Operation operation) {
	Point location = operation.getCur();
	Region& region = m_region_manager.region(std::floor(location.x), std::floor(location.y));
    if (region.get_owner() != m_id) {
		throw std::invalid_argument(u8"区块未被占有");
	}
    if (region.get_building().get_type() == BuildingType::None) {
		throw std::invalid_argument(u8"区块没有建筑");
	}
	//return the cost of the building

	Building& building = region.get_building();
	Config& configer = Config::instance_of();

	std::string building_name = BuildingTypeToString(building.get_type());

	double return_gold = 0;
	double return_oil = 0;
	double return_steel = 0;
	double return_electricity = 0;
	double return_labor = 0;

	int level = building.get_level();
	for (int i = 0; i < level; i++) {
		return_gold += configer.get_building_setting(building_name).ReturnCost[i][ResourceType::GOLD];
		return_oil += configer.get_building_setting(building_name).ReturnCost[i][ResourceType::OIL];
		return_steel += configer.get_building_setting(building_name).ReturnCost[i][ResourceType::STEEL];
		return_electricity += configer.get_building_setting(building_name).ReturnCost[i][ResourceType::ELECTRICITY];
		return_labor += configer.get_building_setting(building_name).ReturnCost[i][ResourceType::LABOR];
	}

	m_resources.gold += return_gold;
	m_resources.oil += return_oil;
	m_resources.steel += return_steel;
	m_resources.electricity += return_electricity;
	m_resources.labor += return_labor;
	region.remove_building();
}

void Player::set_research(Operation operation) {
	Config& configer = Config::instance_of();
	double cost = configer.get_research_institution_setting().cost;
	if (m_resources.gold - m_steady_cost_resources.gold < cost) {
		throw std::invalid_argument(u8"金钱不足");
	}
	m_resources.gold -= cost;
	m_have_research_institution = true;
}

void Player::upgrade_building_level_limit(BuildingType building_type) {
	if(m_institution_level_limit[building_type] == m_max_institution_level[building_type]){
		throw std::invalid_argument(u8"已升级到最高级");
	}
	std::vector<double> cost = Config::instance_of().get_building_setting(BuildingTypeToString(building_type)).BuildCost[m_institution_level_limit[building_type] - 1];
	if (m_resources.gold - m_steady_cost_resources.gold < cost[ResourceType::GOLD]) {
		throw std::invalid_argument(u8"金钱不足");
	}
	if (m_resources.oil - m_steady_cost_resources.oil < cost[ResourceType::OIL]) {
		throw std::invalid_argument(u8"石油不足");
	}
	if (m_resources.electricity - m_steady_cost_resources.electricity < cost[ResourceType::ELECTRICITY]) {
		throw std::invalid_argument(u8"电力不足");
	}
	if (m_resources.steel - m_steady_cost_resources.steel < cost[ResourceType::STEEL]) {
		throw std::invalid_argument(u8"钢铁不足");
	}
	if (m_resources.labor - m_steady_cost_resources.labor < cost[ResourceType::LABOR]) {
		throw std::invalid_argument(u8"劳动力不足");
	}
	m_resources.gold -= cost[ResourceType::GOLD];
	m_resources.oil -= cost[ResourceType::OIL];
	m_resources.steel -= cost[ResourceType::STEEL];
	m_resources.electricity -= cost[ResourceType::ELECTRICITY];
	m_resources.labor -= cost[ResourceType::LABOR];
	m_institution_level_limit[building_type]++;
}

void Player::upgrade_army_level_limit() {
	if(m_army_level[0] == m_max_army_level[0]){
		throw std::invalid_argument(u8"已升级到最高级");
	}
	std::vector<double> cost = Config::instance_of().get_army_parameter().UpLevelCost[m_army_level[0] - 1];
	if (m_resources.gold - m_steady_cost_resources.gold < cost[ResourceType::GOLD]) {
		throw std::invalid_argument(u8"金钱不足");
	}
	m_resources.gold -= cost[ResourceType::GOLD];
	m_army_level[0]++;
}
void Player::upgrade_weapon_level_limit(int weapon_type) {
	if (m_army_level[weapon_type + 1] == m_max_army_level[weapon_type + 1]) {
		throw std::invalid_argument(u8"已升级到最高级");
	}
	std::vector<double> cost = Config::instance_of().get_weapon_parameter(weapon_type).UpLevelCost[m_army_level[weapon_type + 1]];
	if (m_resources.gold - m_steady_cost_resources.gold < cost[ResourceType::GOLD]) {
		throw std::invalid_argument(u8"金钱不足");
	}
	m_resources.gold -= cost[ResourceType::GOLD];
	m_army_level[weapon_type + 1]++;
}

void Player::research(Operation operation) {
	if (!m_have_research_institution) {
		throw std::invalid_argument(u8"未解锁研究所");
	}

    Config& configer = Config::instance_of();

	switch (operation.getOp()) {
	case Operator::PowerStationUpLevel:
		upgrade_building_level_limit(BuildingType::PowerStation);
		break;
	case Operator::RefineryUpLevel:
		upgrade_building_level_limit(BuildingType::Refinery);
		break;
	case Operator::SteelFactoryUpLevel:
		upgrade_building_level_limit(BuildingType::SteelFactory);
		break;
	case Operator::CivilFactoryUpLevel:
		upgrade_building_level_limit(BuildingType::CivilFactory);
		break;
	case Operator::MilitaryFactoryUpLevel:
		upgrade_building_level_limit(BuildingType::MilitaryFactory);
		break;
	case Operator::ArmyUpLevel:
		upgrade_army_level_limit();
		break;
	case Operator::Weapon0UpLevel:
		upgrade_weapon_level_limit(0);
		break;
	case Operator::Weapon1UpLevel:
		upgrade_weapon_level_limit(1);
		break;
	case Operator::Weapon2UpLevel:
		upgrade_weapon_level_limit(2);
		break;
	}
}

void Player::product_weapon(int weapon_type, Region& region) {
	if(m_army_level[weapon_type + 1] == 0){
		throw std::invalid_argument(u8"未解锁该武器");
	}
	Config& configer = Config::instance_of();
	std::vector<int> cost = configer.get_weapon_parameter(weapon_type).cost;
	if (m_resources.gold - m_steady_cost_resources.gold < cost[ResourceType::GOLD]) {
		throw std::invalid_argument(u8"金钱不足");
	}
	if (m_resources.oil - m_steady_cost_resources.oil < cost[ResourceType::OIL]) {
		throw std::invalid_argument(u8"石油不足");
	}
	if (m_resources.electricity - m_steady_cost_resources.electricity < cost[ResourceType::ELECTRICITY]) {
		throw std::invalid_argument(u8"电力不足");
	}
	if (m_resources.steel - m_steady_cost_resources.steel < cost[ResourceType::STEEL]) {
		throw std::invalid_argument(u8"钢铁不足");
	}
	if (m_resources.labor - m_steady_cost_resources.labor < cost[ResourceType::LABOR]) {
		throw std::invalid_argument(u8"劳动力不足");
	}
	m_resources.gold -= cost[ResourceType::GOLD];
	m_resources.steel -= cost[ResourceType::STEEL];
	m_resources.oil -= cost[ResourceType::OIL];
	m_resources.electricity -= cost[ResourceType::ELECTRICITY];
	m_resources.labor -= cost[ResourceType::LABOR];
	
	region.add_weapon_amount(weapon_type);
}

namespace PathFinder {
	std::vector<Point> astar(const Array2D<int>& grid, const Array2D<int> neigbours, Point start, Point end);
}
void Player::product(Operation operation) {
	Point start = operation.getStart();
	Point end = operation.getEnd();

	Region& start_region = m_region_manager.region(std::floor(start.x), std::floor(start.y));
	Region& end_region = m_region_manager.region(std::floor(end.x), std::floor(end.y));

	if (start_region.get_owner() != m_id || end_region.get_owner() != m_id) {
		throw std::invalid_argument(u8"区块未被占有");
	}
	if (start_region.get_building().get_type() != BuildingType::MilitaryFactory) {
		throw std::invalid_argument(u8"没有军事工厂");
	}


	Array2D<int> player_region_matrix(m_region_manager.regions().width(), m_region_manager.regions().height());
	for (int i = 0; i < m_region_manager.regions().width(); i++) {
		for (int j = 0; j < m_region_manager.regions().height(); j++) {
			if (m_region_manager.regions()(i, j).get_owner() == m_id) {
				player_region_matrix(i, j) = 1;
			}
		}
	}
	std::vector<Point> path_points = PathFinder::astar(player_region_matrix, m_region_manager.neighbour_regions(), start, end);

	if (path_points.empty()) {
		throw std::invalid_argument(u8"无法部署到指定位置");
	}

	Config& configer = Config::instance_of();
	int cost{};
	switch (operation.getOp())
	{
	case Operator::ProductArmy:
		cost = configer.get_army_parameter().cost * operation.getSize();
		if (m_resources.gold - m_steady_cost_resources.gold < cost) {
			throw std::invalid_argument(u8"金钱不足");
		}
		m_resources.gold -= cost;
		end_region.add_army_amount(operation.getSize());
		break;
	case Operator::ProductWeapon0:
		product_weapon(0, end_region);
		break;
	case Operator::ProductWeapon1:
		product_weapon(1, end_region);
		break;
	case Operator::ProductWeapon2:
		product_weapon(2, end_region);
		break;
	}
}
void push_error_message(const std::string& msg);
void Player:: update(GlobalTimer& timer){
	std::vector<double> delta_resource = { 0,0,0,0,0 };
	double delta_t = timer.get_dt();
	std::vector<double> steady_cost_resource = { 0,0,0,0,0 };
	m_region_manager.calculate_delta_resources(delta_resource, delta_t, m_id);
	m_region_manager.calculate_steady_cost_resources(steady_cost_resource, m_id);


	struct {
		double gold;
		double oil;
		double steel;
		double electricity;
		double labor;
	} tmp_resources = { m_resources.gold, m_resources.oil, m_resources.steel, m_resources.electricity, m_resources.labor };

	
	tmp_resources.gold += delta_resource[ResourceType::GOLD];
	tmp_resources.oil += delta_resource[ResourceType::OIL];
	tmp_resources.steel += delta_resource[ResourceType::STEEL];
	tmp_resources.electricity += delta_resource[ResourceType::ELECTRICITY];
	tmp_resources.labor += delta_resource[ResourceType::LABOR];


	tmp_resources.labor = m_region_manager.calculate_region_amount(m_id) * 30;



	
	m_steady_cost_resources.gold = steady_cost_resource[ResourceType::GOLD];
	m_steady_cost_resources.oil = steady_cost_resource[ResourceType::OIL];
	m_steady_cost_resources.steel = steady_cost_resource[ResourceType::STEEL];
	m_steady_cost_resources.electricity = steady_cost_resource[ResourceType::ELECTRICITY];
	m_steady_cost_resources.labor = steady_cost_resource[ResourceType::LABOR];

	if (!m_is_power_off) {
		if (tmp_resources.electricity < m_steady_cost_resources.electricity) {
			m_is_power_off = true;
			push_error_message(u8"电力不足，暂停生产");
		}
		if (tmp_resources.gold < m_steady_cost_resources.gold) {
			m_is_power_off = true;
			push_error_message(u8"黄金不足，暂停生产");
		}
		if (tmp_resources.oil < m_steady_cost_resources.oil) {
			m_is_power_off = true;
			push_error_message(u8"石油不足，暂停生产");
		}
		if (tmp_resources.steel < m_steady_cost_resources.steel) {
			m_is_power_off = true;
			push_error_message(u8"钢铁不足，暂停生产");
		}
		if (tmp_resources.labor < m_steady_cost_resources.labor) {
			m_is_power_off = true;
			push_error_message(u8"劳动力不足，暂停生产");
		}

	}
	else {
		if (tmp_resources.gold >= m_steady_cost_resources.gold && tmp_resources.oil >= m_steady_cost_resources.oil && tmp_resources.steel >= m_steady_cost_resources.steel && tmp_resources.electricity >= m_steady_cost_resources.electricity && tmp_resources.labor >= m_steady_cost_resources.labor) {
			m_is_power_off = false;
		}
	}

	if (!m_is_power_off) {
		m_resources.gold = std::fmax(0.0, tmp_resources.gold);
		m_resources.oil = std::fmax(0.0, tmp_resources.oil);
		m_resources.steel = std::fmax(0.0, tmp_resources.steel);
		m_resources.electricity = std::fmax(0.0, tmp_resources.electricity);
		m_resources.labor = std::fmax(0.0, tmp_resources.labor);
	}

}

void Player::range_attack(Operation operation) {
	Point cur = operation.getCur();
	double radius = operation.getRadius();
	int num = radius * radius / 4;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<double> dis(0, radius);
	bool flag = false;
	int cnt = 0;
	std::vector<std::pair<Point, std::vector<int>>> regionWeapons;
	for (int i = 0; i < m_region_manager.map_width(); i++) {
		for (int j = 0; j < m_region_manager.map_height(); j++) {
			Region& region = m_region_manager.region(i, j);
			if (region.get_owner() == m_id) {
				regionWeapons.push_back(std::make_pair(region.get_center_position(), region.get_weapons()));
			}
		}
	}
	while (num) {
		double x = dis(gen);
		double y = dis(gen);
		Point target(x, y);
		int mapSize = m_region_manager.map_width();
		for (auto regionWeapon : regionWeapons) {
			Point regionPos = regionWeapon.first;
			std::vector<int>& weapons = regionWeapon.second;
			double distance = regionPos.distance(target);
			if (weapons[0] > 0) {
				double speed = m_region_manager.get_weapon(0).get_attack_speed(m_army_level[1]);
				/*float damage = regionmanager.get_weapon(0).getDamage(arm_level[1]);*/
				double time = distance / speed;
				if (distance <= 0.25 * mapSize) {
					m_region_manager.attack_region_missle(0, m_army_level[1], regionPos, target, time);
					num--;
					flag = true;
				}
			} else if (weapons[1] > 0) {
				double speed = m_region_manager.get_weapon(1).get_attack_speed(m_army_level[2]);
				/*float damage = regionmanager.get_weapon(1).getDamage(arm_level[2]);*/
				double time = distance / speed;
				if (distance <= 0.5 * mapSize) {
					m_region_manager.attack_region_missle(1, m_army_level[2], regionPos, target, time);
					num--;
					flag = true;
				}
			} else if (weapons[2] > 0) {
				double speed = m_region_manager.get_weapon(2).get_attack_speed(m_army_level[3]);
				/*float damage = regionmanager.get_weapon(2).getDamage(arm_level[3]);*/
				double time = distance / speed;
				if (distance <= 0.75 * mapSize && distance >= 0.2 * mapSize) {
					m_region_manager.attack_region_missle(2, m_army_level[3], regionPos, target, time);
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
	Config& config = Config::instance_of();
	std::tuple<double, double> originSize = config.get_default_region_setting().OriginSize;


	int mapWidth = m_region_manager.map_width();
	int mapHeight = m_region_manager.map_height();
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> disX(3, mapWidth - 3);
	std::uniform_int_distribution<int> disY(3, mapHeight - 3);
	std::uniform_int_distribution<int> disSize(std::get<0>(originSize), std::get<1>(originSize));
	int size = disSize(gen);
	int x = disX(gen);
	int y = disY(gen);
	m_capital_idx = std::make_tuple(x, y);
	double Hp = config.get_default_region_setting().CapitalHP;
	int Force = config.get_default_region_setting().CapitalArmyCount;
	m_region_manager.region(x, y).set_HP(Hp);
	m_region_manager.region(x, y).get_army().add_amount(Force);
	m_region_manager.region(x, y).set_owner(0);
	for (int i = -3; i <= 3; i++) {
		for (int j = -3; j <= 3; j++) {
			if (i * i + j * j > size * size) {
				continue;
			}
			m_region_manager.region(x + i, y + j).set_owner(0);
		}
	}

	m_resources.gold = config.get_player_origion_source().gold;
	m_resources.oil = config.get_player_origion_source().oil;
	m_resources.steel = config.get_player_origion_source().steel;
	m_resources.electricity = config.get_player_origion_source().electricity;
	m_resources.labor = 0;
	m_steady_cost_resources = { 0, 0, 0, 0 ,0 };
	m_army_level = { 1, 0, 0, 0 };
	m_institution_level_limit = { 1, 1, 1, 1, 1 };
	m_have_research_institution = false;
	return;
}

