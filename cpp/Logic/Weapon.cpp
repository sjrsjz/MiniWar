#include "../../header/Logic/Weapon.h"
#include "../../header/utils/Config.h"
#include "../../header/Logic/RegionManager.h"

using json = nlohmann::json;

Weapon::Weapon(int id) {
	//		
	Config& config = Config::getInstance();
	std::string ID = std::to_string(id);
	const json j = config.getConfig({ "Weapon", ID});
	this->damage = j["damage"].get<double>();
	this->damageRange = j["damageRange"].get<double>();
	this->attackSpeed = j["attackSpeed"].get<double>();
	this->attackRange = std::make_tuple(j["AttackRange"][0].get<double>(), j["AttackRange"][1].get<double>());
	this->cost = j["cost"].get<std::vector<int>>();
	this->id = id;
	this->AICost = j["AICost"].get<int>();
}

Weapon::~Weapon() {
}

double Weapon::getDamage(int level) {
	//formula
	double Damage = this->damage * (1 + level * 0.2);
	return Damage;
}

double Weapon::getDamageRange(int level) {
	double DamageRange = this->damageRange * (1 + level * 0.2);
	return DamageRange;
}

double Weapon::getAttackSpeed(int level) {
	double AttackSpeed = this->attackSpeed * (1 + level * 0.2) * RegionManager::getInstance().get_map_width();
	return AttackSpeed; 
}

std::tuple<double, double> Weapon::getAttackRange() {
	return this->attackRange;
}

int Weapon::getId(){
	return this->id;
}

std::vector<int> Weapon::getCost() {
	return this->cost;
}

int Weapon::getAICost() {
	return this->AICost;
}

