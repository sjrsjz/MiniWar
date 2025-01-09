#include "../../header/Logic/Building.h"

Building::Building(BuildingType type) {
	this->type = type;
	this->level = 1;
}

Building::~Building() {
}

bool Building::upLevel(int MaxLevel) {
	if (this->level < MaxLevel) {
		this->level++;
		return true;
	}
	return false;
}

BuildingType Building::getType() {
	return this->type;
}

int Building::getLevel() {
	return this->level;
}

bool Building::remove() {
	if (this->type == BuildingType::None) {
		return false;
	} else {
		this->level = 0;
		this->type = BuildingType::None;
	}
	return true;
}





