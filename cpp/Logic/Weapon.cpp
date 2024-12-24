#include "../../header/Logic/Weapon.h"
#include "../../header/utils/Config.h"

using json = nlohmann::json;

Weapon::Weapon(int id) {
	//		
	Config& config = Config::getInstance();
	std::string ID = std::to_string(id);
	const json j = config.getConfig({ "Weapon", ID});
	this->damage = j["damage"].get<float>();
	this->damageRange = j["damageRange"].get<float>();
	this->attackSpeed = j["attackSpeed"].get<float>();
	this->attackRange = std::make_tuple(j["attackRange"][0].get<float>(), j["attackRange"][1].get<float>());
	this->cost = j["cost"].get<std::vector<int>>();
	this->id = id;
	this->AICost = j["AICost"].get<int>();
}

Weapon::~Weapon() {
}

float Weapon::getDamage(int level){
	//formula
	//
	return this->damage;
}

float Weapon::getDamageRange(int level){
	return this->damageRange;
}

float Weapon::getAttackSpeed(int level){
	return this->attackSpeed;
}

std::tuple<float, float> Weapon::getAttackRange(){
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

