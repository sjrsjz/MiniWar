#pragma once
template<class T>
class array {
private:
	int width, height;
	T* data;
	array(int w, int h);

	T& get(int x, int y);

	~array();
};
