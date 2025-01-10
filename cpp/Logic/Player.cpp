#include "../../header/Logic/Player.h"
#include "../../header/Logic/RegionManager.h"
#include "../../header/debug.h"
#include "../../header/utils/Config.h"
#include "../../header/Logic/Resource.h"
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

int Player::get_building_level_limit(BuildingType type) {
    if (type == BuildingType::PowerStation) return institution_level_limit[BuildingType::PowerStation];
	if (type == BuildingType::Refinery) return institution_level_limit[BuildingType::Refinery];
    if (type == BuildingType::SteelFactory) return institution_level_limit[BuildingType::SteelFactory];
	if (type == BuildingType::CivilFactory) return institution_level_limit[BuildingType::CivilFactory];
	if (type == BuildingType::MilitaryFactory) return institution_level_limit[BuildingType::MilitaryFactory];
	return -1;
}

void Player:: move_army(Operation operation, int amount){

	Point start = operation.getStart();
	Point end = operation.getEnd();

    int start_x = std::floor(start.x);
    int start_y = std::floor(start.y);
    int end_x = std::floor(end.x);
    int end_y = std::floor(end.y);

	Region& start_region = regionmanager.get_region(start_x, start_y);
	Region& end_region = regionmanager.get_region(end_x, end_y);

	if (start_region.getOwner() != id) {
		throw std::invalid_argument(u8"区块未被占有");
	}
	if (start_region.getArmy().getForce() < amount || amount == 0) {
		throw std::invalid_argument(u8"军队数量不足");
	}

	regionmanager.move_army(start, end, amount, army_level[0]);
}

void Player::attack(Operation operation) {
	Point start = operation.getStart();
	Point end = operation.getEnd();
	double distance_origin = start.distance(end);
	double distance = distance_origin / fmax(regionmanager.get_map_width(), regionmanager.get_map_height());

    int start_x = std::floor(start.x);
    int start_y = std::floor(start.y);

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

	std::tuple<double, double> AttackRange = weapon.getAttackRange();
	if (start_region.getWeapons()[weapon_id] <= 0) {
		throw std::invalid_argument(u8"武器数量不足");
	}

	if (distance > std::get<1>(AttackRange) || distance < std::get<0>(AttackRange)) {
		throw std::invalid_argument(u8"超出攻击范围");
	}

    double time = distance_origin / weapon.getAttackSpeed(army_level[id+1]);

	start_region.removeWeapon(weapon_id);

	regionmanager.attack_region_missle(weapon_id, army_level[weapon_id + 1], start, end, time);
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

	Region& region = regionmanager.get_region(std::floor(location.x), std::floor(location.y));
	if (region.getOwner() != id) {
		throw std::invalid_argument(u8"区块未被占有");
	}
    if (region.getBuilding().getType() != BuildingType::None) {
		throw std::invalid_argument(u8"已放置建筑");
	}
	std::vector<double> Level1Cost = Config::getInstance().getBuildingSetting(BuildingTypeToString(building_type)).BuildCost;

	gold -= Level1Cost[ResourceType::GOLD];
	oil -= Level1Cost[ResourceType::OIL];
	steel -= Level1Cost[ResourceType::STEEL];
	electricity -= Level1Cost[ResourceType::ELECTRICITY];
	Building building(building_type);
	region.setBuilding(building);
}

void Player::upgrade_building(Operation operation) {
	Point location = operation.getCur();
	Region& region = regionmanager.get_region(std::floor(location.x), std::floor(location.y));
    if (region.getBuilding().getType() == BuildingType::None) {
        throw std::invalid_argument(u8"当前区块没有建筑");
    }
	if (region.getOwner() != id) {
		throw std::invalid_argument(u8"区块未被占有");
	}

    if (region.getBuilding().getLevel() == institution_level_limit[get_building_level_limit(region.getBuilding().getType())]) {
        throw std::invalid_argument(u8"已升级到最高级");
    }

	Config& configer = Config::getInstance();

	std::string building_name = BuildingTypeToString(region.getBuilding().getType());
	int level = region.getBuilding().getLevel();
	gold -= configer.getBuildingSetting(building_name).UpLevelCost[level][ResourceType::GOLD];
	oil -= configer.getBuildingSetting(building_name).UpLevelCost[level][ResourceType::OIL];
	steel -= configer.getBuildingSetting(building_name).UpLevelCost[level][ResourceType::STEEL];
	electricity -= configer.getBuildingSetting(building_name).UpLevelCost[level][ResourceType::ELECTRICITY];
	ocupied_labor += configer.getBuildingSetting(building_name).UpLevelCost[level][ResourceType::LABOR];


	region.getBuilding().upLevel(institution_level_limit[region.getBuilding().getType()]);
}

void Player::remove_building(Operation operation) {
	Point location = operation.getCur();
	Region& region = regionmanager.get_region(std::floor(location.x), std::floor(location.y));
    if (region.getOwner() != id) {
		throw std::invalid_argument(u8"区块未被占有");
	}
    if (region.getBuilding().getType() == BuildingType::None) {
		throw std::invalid_argument(u8"区块没有建筑");
	}
	//return the cost of the building

	Building& building = region.getBuilding();
	Config& configer = Config::getInstance();

	std::string building_name = BuildingTypeToString(building.getType());

	double return_gold = configer.getBuildingSetting(building_name).BuildCost[ResourceType::GOLD];
	double return_oil = configer.getBuildingSetting(building_name).BuildCost[ResourceType::OIL];
	double return_steel = configer.getBuildingSetting(building_name).BuildCost[ResourceType::STEEL];

	int level = building.getLevel();
	for (int i = 0; i < level - 1; i++) {
		return_gold += configer.getBuildingSetting(building_name).UpLevelCost[i][ResourceType::GOLD];
		return_oil += configer.getBuildingSetting(building_name).UpLevelCost[i][ResourceType::OIL];
		return_steel += configer.getBuildingSetting(building_name).UpLevelCost[i][ResourceType::STEEL];
	}

	double return_factor = 0.6;
	gold += return_gold * return_factor;
	oil += return_oil * return_factor;
	steel += return_steel * return_factor;
	region.removeBuilding();
}

void Player::set_research(Operation operation) {
	Config& configer = Config::getInstance();
	double cost = configer.getResearchInstitutionSetting().cost;
	if (gold < cost) {
		throw std::invalid_argument(u8"金钱不足");
	}
	gold -= cost;
	have_research_institution = true;
}

void Player::upgrade_building_level_limit(BuildingType building_type) {
	if(institution_level_limit[building_type] == max_institution_level[building_type]){
		throw std::invalid_argument(u8"已升级到最高级");
	}
	std::vector<double> cost = Config::getInstance().getBuildingSetting(BuildingTypeToString(building_type)).UpLevelCost[institution_level_limit[building_type] - 1];
	if (gold < cost[ResourceType::GOLD]) {
		throw std::invalid_argument(u8"金钱不足");
	}
	if (oil < cost[ResourceType::OIL]) {
		throw std::invalid_argument(u8"石油不足");
	}
	if (electricity < cost[ResourceType::ELECTRICITY]) {
		throw std::invalid_argument(u8"电力不足");
	}
	if (steel < cost[ResourceType::STEEL]) {
		throw std::invalid_argument(u8"钢铁不足");
	}
	gold -= cost[0];
	oil -= cost[1];
	electricity -= cost[2];
	steel -= cost[3];
	institution_level_limit[building_type]++;
}

void Player::upgrade_army_level_limit() {
	if(army_level[0] == max_army_level[0]){
		throw std::invalid_argument(u8"已升级到最高级");
	}
	std::vector<double> cost = Config::getInstance().getArmy().UpLevelCost[army_level[0] - 1];
	if (gold < cost[ResourceType::GOLD]) {
		throw std::invalid_argument(u8"金钱不足");
	}
	gold -= cost[ResourceType::GOLD];
	army_level[0]++;
}
void Player::upgrade_weapon_level_limit(int weapon_type) {
	if (army_level[weapon_type + 1] == max_army_level[weapon_type + 1]) {
		throw std::invalid_argument(u8"已升级到最高级");
	}
	std::vector<double> cost = Config::getInstance().getWeapon(weapon_type).UpLevelCost[army_level[weapon_type + 1] - 1];
	if (gold < cost[ResourceType::GOLD]) {
		throw std::invalid_argument(u8"金钱不足");
	}
	gold -= cost[ResourceType::GOLD];
	army_level[weapon_type + 1]++;
}

void Player::research(Operation operation) {
	if (!have_research_institution) {
		throw std::invalid_argument(u8"未解锁研究所");
	}

    Config& configer = Config::getInstance();

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
	if(army_level[weapon_type + 1] == 0){
		throw std::invalid_argument(u8"未解锁该武器");
	}
	Config& configer = Config::getInstance();
	std::vector<int> cost = configer.getWeapon(weapon_type).cost;
	if (gold < cost[ResourceType::GOLD]) {
		throw std::invalid_argument(u8"金钱不足");
	}
	if (oil < cost[ResourceType::OIL]) {
		throw std::invalid_argument(u8"石油不足");
	}
	if (electricity < cost[ResourceType::ELECTRICITY]) {
		throw std::invalid_argument(u8"电力不足");
	}
	if (steel < cost[ResourceType::STEEL]) {
		throw std::invalid_argument(u8"钢铁不足");
	}
	if (labor_limit - ocupied_labor < cost[ResourceType::LABOR]) {
		throw std::invalid_argument(u8"劳动力不足");
	}
	gold -= cost[ResourceType::GOLD];
	oil -= cost[ResourceType::OIL];
	steel -= cost[ResourceType::STEEL];

	
	region.addWeapon(weapon_type);
}

void Player::product(Operation operation) {
	Point start = operation.getStart();
	Point end = operation.getEnd();
	Region& start_region = regionmanager.get_region(std::floor(start.x), std::floor(start.y));
	Region& end_region = regionmanager.get_region(std::floor(end.x), std::floor(end.y));

	if (start_region.getOwner() != id || end_region.getOwner() != id) {
		throw std::invalid_argument(u8"区块未被占有");
	}
	if (start_region.getBuilding().getType() != BuildingType::MilitaryFactory) {
		throw std::invalid_argument(u8"没有军事工厂");
	}
	Config& configer = Config::getInstance();
	int cost{};
	std::vector<int> cost0;
	switch (operation.getOp())
	{
	case Operator::ProductArmy:
		cost = configer.getArmy().cost * operation.getSize();
		if (gold < cost) {
			throw std::invalid_argument(u8"金钱不足");
		}
		gold -= cost;
		end_region.addArmy(operation.getSize());
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
	double radius = operation.getRadius();
	int num = radius * radius / 4;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<double> dis(0, radius);
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
		double x = dis(gen);
		double y = dis(gen);
		Point target(x, y);
		int mapSize = regionmanager.get_map_width();
		for (auto regionWeapon : regionWeapons) {
			Point regionPos = regionWeapon.first;
			std::vector<int>& weapons = regionWeapon.second;
			double distance = regionPos.distance(target);
			if (weapons[0] > 0) {
				double speed = regionmanager.get_weapon(0).getAttackSpeed(army_level[1]);
				/*float damage = regionmanager.get_weapon(0).getDamage(arm_level[1]);*/
				double time = distance / speed;
				if (distance <= 0.25 * mapSize) {
					regionmanager.attack_region_missle(0, army_level[1], regionPos, target, time);
					num--;
					flag = true;
				}
			} else if (weapons[1] > 0) {
				double speed = regionmanager.get_weapon(1).getAttackSpeed(army_level[2]);
				/*float damage = regionmanager.get_weapon(1).getDamage(arm_level[2]);*/
				double time = distance / speed;
				if (distance <= 0.5 * mapSize) {
					regionmanager.attack_region_missle(1, army_level[2], regionPos, target, time);
					num--;
					flag = true;
				}
			} else if (weapons[2] > 0) {
				double speed = regionmanager.get_weapon(2).getAttackSpeed(army_level[3]);
				/*float damage = regionmanager.get_weapon(2).getDamage(arm_level[3]);*/
				double time = distance / speed;
				if (distance <= 0.75 * mapSize && distance >= 0.2 * mapSize) {
					regionmanager.attack_region_missle(2, army_level[3], regionPos, target, time);
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
	Config& config = Config::getInstance();
	std::tuple<double, double> originSize = config.getDefaultRegionSetting().OriginSize;


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
	double Hp = config.getDefaultRegionSetting().CapitalHP;
	int Force = config.getDefaultRegionSetting().CapitalArmyCount;
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

	gold = config.getPlayerOrigionSource().gold;
	oil = config.getPlayerOrigionSource().oil;
	electricity = config.getPlayerOrigionSource().electricity;
	steel = config.getPlayerOrigionSource().steel;
	army_level = { 1, 0, 0, 0 };
	institution_level_limit = { 1, 1, 1, 1, 1 };
	have_research_institution = false;

	return;
}

