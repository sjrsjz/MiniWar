#include "../../header/Logic/Army.h"

Army::Army(int force, int level) {
	this->m_level = level;
	this->m_force = force;
	/* speed = */ 
}

Army::~Army() {
}

double Army::get_speed() {
	return this->m_speed;
}

int Army::get_level(){
	return this->m_level;
}

int Army::get_force(){
	return this->m_force;
}

bool Army::add_amount(int num){
	this->m_force += num;
	return true;
}

bool Army::reduce_amount(int num){
	if (this->m_force < num) {
		return false;
	}
	this->m_force -= num;
	this->m_force = this->m_force < 0 ? 0 : this->m_force;
	return true;
}
void Army::set_amount(int num) {
	this->m_force = num;
}