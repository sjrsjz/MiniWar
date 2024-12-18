#include "../../header/Logic/Army.h"

Army::Army(int force, int level) {
	this->level = level;
	this->force = force;
	/* speed = */ 
}

Army::~Army() {
}

float Army::getSpeed(){
	return this->speed;
}

int Army::getLevel(){
	return this->level;
}

int Army::getForce(){
	return this->force;
}

bool Army::addArmy(int num){
	this->force += num;
	return true;
}

bool Army::removeArmy(int num){
	if (this->force < num) {
		return false;
	}
	this->force -= num;
	return true;
}
