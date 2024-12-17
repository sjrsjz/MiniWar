#include "../../header/utils/Array.h"

template<class T>
Array<T>::Array(int w, int h) {
	width = w;
	height = h;
	this->data = new T[w * h];
}

template<class T>
Array<T>::~Array() {
	delete[] data;
}

template<class T>
T& Array<T>::get(int x, int y) {
	if (x >= this->width || y >= this->height) {
		throw std::out_of_range("Index out of range");
	}
	if (x < 0 || y < 0) {
		throw std::out_of_range("Index should larger than 0");
	}
	return this->data[x + y * this->width];
}

template<class T>
int Array<T>::get_width() const{
	return this->width;
}

template<class T>
int Array<T>::get_height() const{
	return this->height;
}