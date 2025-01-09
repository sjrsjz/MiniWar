#include "../../header/utils/Point.h"

Point::Point() {
	this->x = 0;
	this->y = 0;
}

Point::Point(double x, double y) {
	this->x = x;
	this->y = y;
}

Point::~Point() {
}

double Point::distance(Point& p) {
	return sqrt(pow(this->x - p.x, 2) + pow(this->y - p.y, 2));
}
bool Point::operator==(const Point& p) {
	return this->x == p.x && this->y == p.y;
}
bool Point::operator!=(const Point& p) {
	return this->x != p.x || this->y != p.y;
}
Point Point::operator+(const Point& p) {
	return Point(this->x + p.x, this->y + p.y);
}