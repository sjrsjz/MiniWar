#pragma once
#include <iostream>
template<class T>
class Array {
private:
	int width;
	int height;
	T* data;
public:
	Array(int w, int h);
	~Array();
	T& get(int x, int y);
};
