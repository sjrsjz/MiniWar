#pragma once
#include <cmath>

class Point {
public:
	double x, y;
	Point();
	Point(double x, double y);
	~Point();
	double distance(Point& p);
	bool operator==(const Point& p);
	bool operator!=(const Point& p);
	Point operator+(const Point& p);
	static Point toPoint(int p[2]) {
		return Point(p[0], p[1]);
	}
};
