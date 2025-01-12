#pragma once
#ifndef _Timer_h
#define _Timer_h
// 计时器，以秒为单位
class Timer {
private:
	double lt = 0;
public:
	double dt = 0;
	Timer() {
	}
	Timer(double t) { lt = t; }
	void set_time(double t) {
		dt = t - lt; lt = t;
	}
	double time() const{
		return lt;
	}
};
#endif