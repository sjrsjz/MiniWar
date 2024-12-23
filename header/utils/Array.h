#pragma once
#include <iostream>

template<class T>
class Array {
private:
	int width;
	int height;
	T* data;
public:

	inline T& operator()(int x, int y) {
		return get(x, y);
	}


	Array() {
		width = 0;
		height = 0;
		this->data = nullptr;
	}

	Array(int w, int h) {
		width = w;
		height = h;
		this->data = new T[w * h];
	}

	~Array() {
		delete[] data;
	}

	T& get(int x, int y) {
		if (x >= this->width || y >= this->height) {
			throw std::out_of_range("Index out of range");
		}
		if (x < 0 || y < 0) {
			throw std::out_of_range("Index should larger than 0");
		}
		return this->data[x + y * this->width];
	}


	int get_width() const {
		return this->width;
	}

	int get_height() const {
		return this->height;
	}

};
