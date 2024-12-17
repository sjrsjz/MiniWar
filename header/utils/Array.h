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
	int get_width() const;
	int get_height() const;
	inline T& operator()(int x, int y) {
		return get(x, y);
	}
};
