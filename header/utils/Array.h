#pragma once
<<<<<<< HEAD
#include<iostream>
=======
#include <iostream>
>>>>>>> 342fef05818501dee7608a80ac7adf078302a9c5

template<class T>
class Array {
private:
	int width;
<<<<<<< HEAD
	int hight;
	T** data;
=======
	int height;
	T* data;
>>>>>>> 342fef05818501dee7608a80ac7adf078302a9c5
	Array(int w, int h);

	~Array();

public:
	T& get(int x, int y);


};
