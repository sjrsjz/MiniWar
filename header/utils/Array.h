#pragma once
#include <iostream>
<<<<<<< HEAD
=======

>>>>>>> 5d7c84a7f2fab7bbd1cf56dfc619eca428cf0272
template<class T>
class Array {
private:
	int width;
	int height;
	T* data;
<<<<<<< HEAD
=======
	Array(int w, int h);

	~Array();

>>>>>>> 5d7c84a7f2fab7bbd1cf56dfc619eca428cf0272
public:
	Array(int w, int h);
	~Array();
	T& get(int x, int y);
};
