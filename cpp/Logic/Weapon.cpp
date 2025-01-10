#include "../../header/Logic/Weapon.h"
#include "../../header/utils/Config.h"
#include "../../header/Logic/RegionManager.h"

using json = nlohmann::json;

Weapon::Weapon(int id) {
	//		
	Config& config = Config::getInstance();
	this->damage = config.getWeapon(id).damage;
	this->damageRange = config.getWeapon(id).damageRange;
	this->attackSpeed = config.getWeapon(id).attackSpeed;
	this->attackRange = config.getWeapon(id).attackRange;
	this->cost = config.getWeapon(id).cost;
	this->id = id;
	this->AICost = config.getWeapon(id).AICost;
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

