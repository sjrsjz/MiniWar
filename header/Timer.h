#pragma once
#ifndef _Timer_h
#define _Timer_h
// 计时器，以秒为单位
class Timer {
private:
	double lt = 0;
public:
	double dt = 0;
	Timer(double t) { lt = t; }
	void setTime(double t) {
		dt = t - lt; lt = t;
	}
	double getTime() {
		return lt;
	}
};
#endif