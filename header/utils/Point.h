#pragma once
#include <cmath>

class Point {
	float x, y;

public:
	Point();
	Point(float x, float y);
	~Point();
	float getX();
	float getY();
	void setX(float x);
	void setY(float y);
	float distance(Point& p);
	bool operator==(const Point& p);
	static Point toPoint(int p[2]) {
		return Point(p[0], p[1]);
	}
};
