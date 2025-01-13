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

double Point::distance(const Point& p) {
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

Point Point::operator-(const Point& p) {
	return Point(this->x - p.x, this->y - p.y);
}

Point Point::operator*(double d) {
	return Point(this->x * d, this->y * d);
}

Point Point::operator/(double d) {
	return Point(this->x / d, this->y / d);
}

double Point::length() {
	return sqrt(this->x * this->x + this->y * this->y);
}

double Point::distancesq(const Point& p) {
	return (this->x - p.x) * (this->x - p.x) + (this->y - p.y) * (this->y - p.y);
}