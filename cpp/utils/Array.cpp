#include "../../header/utils/Array.h"
<<<<<<< HEAD
//#include "../header/utils/Array.h"
=======
>>>>>>> 342fef05818501dee7608a80ac7adf078302a9c5

template<class T>
Array<T>::Array(int w, int h) {
	width = w;
<<<<<<< HEAD
	hight = h;
	data = new T * [w];
	for (size_t i = 0; i < w; i++) {
		data[i] = new T[h];
	}
=======
	height = h;
	this->data = new T[w * h];
>>>>>>> 342fef05818501dee7608a80ac7adf078302a9c5
}

template<class T>
Array<T>::~Array() {
<<<<<<< HEAD
	for (size_t i = 0; i < width; i++) {
		delete[] data[i];
	}
	delete[] data;
=======
	delete[] this->data;
>>>>>>> 342fef05818501dee7608a80ac7adf078302a9c5
}

template<class T>
T& Array<T>::get(int x, int y) {
<<<<<<< HEAD
	if (x >= this->width || y >= this->hight) {
=======
	if (x >= this->width || y >= this->height) {
>>>>>>> 342fef05818501dee7608a80ac7adf078302a9c5
		throw std::out_of_range("Index out of range");
	}
	if (x < 0 || y < 0) {
		throw std::out_of_range("Index should larger than 0");
	}
<<<<<<< HEAD
	return data[x][y];
=======
	return this->data[x + y * this->width];
>>>>>>> 342fef05818501dee7608a80ac7adf078302a9c5
}