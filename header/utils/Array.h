#pragma once

template<class T>
class Array {
private:
	int width;
	int hight;
	T** data;
	Array(int w, int h);

	~Array();

public:
	T& get(int x, int y);


};
