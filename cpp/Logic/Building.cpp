#include "../../header/Logic/Building.h"

Building::Building(std::string name) {
	this->name = name;
	//TODO
	//this->production = ?
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

std::string Building::getName() {
	return this->name;
}

bool Building::remove() {
	if (this->name == "none"){
		return false;
	} else {
		this->name = "none";
	}
	return true;
}





