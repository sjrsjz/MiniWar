#pragma once
#include <cmath>

class Point {
public:
	double x, y;
	Point();
	Point(double x, double y);
	~Point();
	bool operator==(const Point& p);
	bool operator!=(const Point& p);
	static Point to_point(int p[2]) {
		return Point(p[0], p[1]);
	}
	Point operator-(const Point& p);
	Point operator*(double d);
	Point operator/(double d);
	Point operator+(const Point& p);
	double distance(const Point& p);
	double distancesq(const Point& p);
	double length();
};
