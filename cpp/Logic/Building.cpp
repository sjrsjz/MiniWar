#include "../../header/Logic/Building.h"

Building::Building(BuildingType type) {
	this->m_type = type;
	this->m_level = 1;
}

Building::~Building() {
}

bool Building::up_level(int MaxLevel) {
	if (this->m_level < MaxLevel) {
		this->m_level++;
		return true;
	}
	return false;
}

BuildingType Building::get_type() {
	return this->m_type;
}

int Building::get_level() {
	return this->m_level;
}

bool Building::remove() {
	if (this->m_type == BuildingType::None) {
		return false;
	} else {
		this->m_level = 0;
		this->m_type = BuildingType::None;
	}
	return true;
}





