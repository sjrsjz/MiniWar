#include "../header/utils/Array.h"

template<class T>
Array<T>::Array(int w, int h) {
	width = w;
	hight = h;
	data = new T * [w];
	for (size_t i = 0; i < w; i++) {
		data[i] = new T[h];
	}
}

template<class T>
Array<T>::~Array() {
	delete[] data;
}

template<class T>
T& Array<T>::get(int x, int y) {
	if (x >= this->width || y >= this->hight) {
		throw std::out_of_range("Index out of range");
	}
}