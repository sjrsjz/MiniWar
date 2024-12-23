#include "../../header/Logic/Region.h"
#include "../../header/Exception/FullHpException.h"
#include "../../header/Exception/SurrenderNotAttackedException.h"
#include "../../header/utils/Config.h"
#include "../../header/Logic/RegionManager.h"
#include <sstream>
#include <type_traits>
#include <random>
#include <tuple>

Region::Region() {
	Config& config = Config::getInstance();
	std::tuple<int, int> hpRange = config.getConfig({ "Region", "hp" }).template get<std::tuple<int ,int>>();
	std::tuple<int, int> armyRange = config.getConfig({ "Region", "Army" }).template get<std::tuple<int ,int>>();

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0.0, 1.0);
	float X = dis(gen);
	float Y = dis(gen);
	this->owner = -1;
	this->weapons = std::vector<int>(3, 0);
	this->position = Point(X, Y);
	std::uniform_int_distribution<> disHp(std::get<0>(hpRange), std::get<1>(hpRange));
	std::uniform_int_distribution<> disArmy(std::get<0>(armyRange), std::get<1>(armyRange));
	int armyForce = disArmy(gen);
	this->army = Army(armyForce);
	this->hp = disHp(gen);
	this->maxHp = this->hp;
}

Region::Region(int x, int y){
	//TODO
	//this->hp = maxHp
	//this->maxHp = ?;
	Config config = Config::getInstance();
	std::tuple<int, int> hpRange = config.getConfig({ "region", "hp" }).template get<std::tuple<int ,int>>();

	
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0.0, 1.0);
	float X = dis(gen);
	float Y = dis(gen);
	this->owner = -1; 
	this->weapons = std::vector<int>(3, 0);
	this->position = Point(X + x, Y + y);
	std::uniform_int_distribution<> disHp(std::get<0>(hpRange), std::get<1>(hpRange));
	this->hp = disHp(gen);
	this->maxHp = this->hp;
}

Region::~Region() {
}

bool Region::setOwner(int owner) {
	this->owner = owner;
	return true;
}

int Region::getOwner() {
	return this->owner;
} 

bool Region::increaseHp(float hp) {
	if (this->hp == this->maxHp) {
		float x = position.getX();
		float y = position.getY();
		std::stringstream s;
		
		s << "Region at (" << x << ", " << y << ") is already at full hp";
		throw FullHpException(s.str());
	}
	this->hp = std::max(this->hp + hp, this->maxHp);
	return true;
}

bool Region::decreaseHp(float hp) {
	if (this->hp == 0) {
		float x = position.getX();
		float y = position.getY();
		std::stringstream s;
		s << "Region at (" << x << ", " << y << ") is already at 0 hp";
		throw SurrenderNotAttackedException(s.str());
	}
	
	this->hp = std::min(this->hp - hp, 0.0f);
	if (this->hp == 0) {
		this->owner = -1;
	}
	return true;
}

float Region::getHp() {
	return this->hp;
}

bool Region::setBuilding(Building& building) {
	this->building = building;
	return true;
}

Building& Region::getBuilding() {
	return this->building;
}

Point Region::getPosition() {
	return this->position;
}

bool Region::addWeapon(int weapon, int num) {
	try {
		this->weapons.at(weapon) += num;
	} catch (std::out_of_range& e) {
		throw;
	}
	return true;
}

bool Region::removeWeapon(int weapon) {

	try {
		if (this->weapons.at(weapon) == 0) {
			return false;
		}
		this->weapons.at(weapon)--;
	} catch (std::out_of_range& e) {
		throw;
	}
	return true;
}

std::vector<int> Region::getWeapons() {
	return this->weapons;
}

bool Region::removeArmy(int num) {
	return this->army.removeArmy(num);
}

bool Region::addArmy(int num) {
	return this->army.addArmy(num);
}

Army& Region::getArmy() {
	return this->army;
}

bool Region::removeBuilding() {
	//TODO
	return this->building.remove();
}

bool Region::setHp(float hp) {
	this->hp = hp;
	return true;
}

bool Region::setMaxHp(float maxHp) {
	this->maxHp = maxHp;
	return true;
}
