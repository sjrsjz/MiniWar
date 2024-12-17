#include "../../header/utils/Point.h"

Point::Point() {
	this->x = 0;
	this->y = 0;
}

Point::Point(float x, float y) {
	this->x = x;
	this->y = y;
}

Point::~Point() {
}

float Point::distance(Point& p) {
	return sqrt(pow(this->x - p.x, 2) + pow(this->y - p.y, 2));
}

float Point::getX() {
	return this->x;
}

float Point::getY() {
	return this->y;
}

void Point::setX(float x) {
	this->x = x;
}

void Point::setY(float y) {
	this->y = y;
}
