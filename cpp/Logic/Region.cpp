#include "../../header/Logic/Region.h"
#include "../../header/Exception/FullHpException.h"
#include "../../header/Exception/SurrenderNotAttackedException.h"
#include "../../header/utils/Config.h"
#include "../../header/Logic/RegionManager.h"
#include "../../header/debug.h"
#include <sstream>
#include <type_traits>
#include <random>
#include <tuple>

Region::Region() {
	Config& config = Config::instance_of();
	std::tuple<double, double> hpRange = config.get_default_region_setting().HP;
	std::tuple<double, double> armyRange = config.get_default_region_setting().ArmyCount;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0.0, 1.0);
	double X = dis(gen);
	double Y = dis(gen);
	this->m_owner = -1;
	this->m_weapons = std::vector<int>(3, 0);
	this->m_position = Point(X, Y);
	std::uniform_int_distribution<> disHp(std::get<0>(hpRange), std::get<1>(hpRange));
	std::uniform_int_distribution<> disArmy(std::get<0>(armyRange), std::get<1>(armyRange));
	int armyForce = disArmy(gen);
	this->m_army = Army(armyForce);
	this->m_HP = disHp(gen);
	this->m_max_HP = this->m_HP;
}

Region::Region(int x, int y){
	Config& config = Config::instance_of();
	std::tuple<double, double> hpRange = config.get_default_region_setting().HP;
	std::tuple<double, double> armyRange = config.get_default_region_setting().ArmyCount;
	
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0.0, 1000);
	double X = dis(gen) / 1000.0;
	double Y = dis(gen) / 1000.0;
	this->m_owner = -1; 
	this->m_weapons = std::vector<int>(3, 0);
	this->m_position = Point(X + x, Y + y);
	DEBUGOUTPUT("X", X + x, "Y", Y + y);
	std::uniform_int_distribution<> disHp(std::get<0>(hpRange), std::get<1>(hpRange));
	std::uniform_int_distribution<> disArmy(std::get<0>(armyRange), std::get<1>(armyRange));
	int armyForce = disArmy(gen);
	this->m_army = Army(armyForce);

	this->m_HP = disHp(gen);
	this->m_max_HP = this->m_HP;
}

Region::~Region() {
}

bool Region::set_owner(int owner) {
	this->m_owner = owner;
	return true;
}

int Region::get_owner() {
	return this->m_owner;
} 

bool Region::increase_HP(double hp) {
	if (this->m_HP == this->m_max_HP) {
		double x = m_position.x;
		double y = m_position.y;
		std::stringstream s;
		
		s << "Region at (" << x << ", " << y << ") is already at full hp";
		DEBUGOUTPUT("Region::increaseHp() throws");
		throw FullHpException(s.str());
	}
	this->m_HP = std::max(this->m_HP + hp, this->m_max_HP);
	return true;
}

bool Region::decrease_HP(double hp) {
	if (this->m_HP == 0) {
		double x = m_position.x;
		double y = m_position.y;
		std::stringstream s;
		s << "Region at (" << x << ", " << y << ") is already at 0 hp";
		DEBUGOUTPUT("Region::decreaseHp() throws");
		throw SurrenderNotAttackedException(s.str());
	}
	
	this->m_HP = std::min(this->m_HP - hp, 0.0);
	if (this->m_HP == 0) {
		this->m_owner = -1;
	}
	return true;
}

double Region::get_HP() {
	return this->m_HP;
}

bool Region::set_building(Building& building) {
	this->m_building = building;
	return true;
}

Building& Region::get_building() {
	return this->m_building;
}

Point Region::get_center_position() {
	return this->m_position;
}

bool Region::add_weapon_amount(int weapon, int num) {
	try {
		this->m_weapons.at(weapon) += num;
	} catch (std::out_of_range& e) {
		DEBUGOUTPUT("Region::addWeapon() throws");
		throw e;
	}
	return true;
}

bool Region::remove_weapon(int weapon) {

	try {
		if (this->m_weapons.at(weapon) == 0) {
			return false;
		}
		this->m_weapons.at(weapon)--;
	} catch (std::out_of_range& e) {
		DEBUGOUTPUT("Region::addWeapon() throws");
		throw e;
	}
	return true;
}

std::vector<int> Region::get_weapons() {
	return this->m_weapons;
}

bool Region::reduce_army_amount(int num) {
	return this->m_army.reduce_amount(num);
}

bool Region::add_army_amount(int num) {
	return this->m_army.add_amount(num);
}

Army& Region::get_army() {
	return this->m_army;
}

bool Region::remove_building() {
	//TODO
	return this->m_building.remove();
}

bool Region::set_HP(double hp) {
	this->m_HP = hp;
	return true;
}

bool Region::set_max_HP(double maxHp) {
	this->m_max_HP = maxHp;
	return true;
}
