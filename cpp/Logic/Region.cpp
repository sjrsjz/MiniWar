#include "../../header/Logic/Region.h"
#include "../../header/Exception/FullHpException.h"
#include "../../header/Exception/SurrenderNotAttackedException.h"
#include <sstream>
#include <type_traits>



Region::Region(Point position) {
	//TODO
	//this->hp = maxHp
	//this->maxHp = ?;
	this->owner = -1; 
	this->weapons = std::vector<int>(4, 0);
	this->position = position;
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

bool Region::addWeapon(int weapon, int num = 1) {
	try {
		this->weapons.at(weapon) += num;
	} catch (std::out_of_range& e) {
		throw;
	}
	return true;
}

bool Region::removeWeapon(int weapon, int num = 1) {
	if (this->weapons.at(weapon) < num) {
		return false;
	}

	try {
		this->weapons.at(weapon) -= num;
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

