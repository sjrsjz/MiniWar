#include "../../header/Logic/Weapon.h"
//#include "../../header/utils/Config.h"

Weapon::Weapon(int id) {
	//		
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

std::tuple<float, float> Weapon::getAttackRange(int level){
	return this->attackRange;
}

int Weapon::getId(){
	return this->id;
}

