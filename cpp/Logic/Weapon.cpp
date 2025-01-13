#include "../../header/Logic/Weapon.h"
#include "../../header/utils/Config.h"
#include "../../header/Logic/RegionManager.h"

using json = nlohmann::json;

Weapon::Weapon(int id) {
	//		
	Config& config = Config::instance_of();
	this->m_damage = config.get_weapon_parameter(id).damage;
	this->m_damage_range = config.get_weapon_parameter(id).damageRange;
	this->m_attack_speed = config.get_weapon_parameter(id).attackSpeed;
	this->m_attack_range = config.get_weapon_parameter(id).attackRange;
	this->m_cost = config.get_weapon_parameter(id).cost;
	this->m_id = id;
	this->m_AI_cost = config.get_weapon_parameter(id).AICost;
}

Weapon::~Weapon() {
}

double Weapon::get_damage(int level) {
	//formula
	double Damage = this->m_damage * (1 + level * 0.2);
	return Damage;
}

double Weapon::get_damage_range(int level) {
	double DamageRange = this->m_damage_range * (1 + level * 0.2);
	return DamageRange;
}

double Weapon::get_attack_speed(int level) {
	double AttackSpeed = this->m_attack_speed * (1 + level * 0.2) * RegionManager::instance_of().map_width();
	return AttackSpeed; 
}

std::tuple<double, double> Weapon::get_attack_range() {
	return this->m_attack_range;
}

int Weapon::get_id(){
	return this->m_id;
}

std::vector<int> Weapon::get_cost() {
	return this->m_cost;
}

int Weapon::get_AI_cost() {
	return this->m_AI_cost;
}

