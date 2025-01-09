﻿#pragma once
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

	inline const T& operator()(int x, int y) const {
		return get(x, y);
	}
	Array() {
		width = 0;
		height = 0;
		this->data = nullptr;
	}

	Array(const Array& other) {
		width = other.width;
		height = other.height;
		if (width * height > 0) {
			this->data = new T[width * height];
			for (int i = 0; i < width * height; i++) {
				this->data[i] = other.data[i];
			}
		}
		else {
			this->data = nullptr;
		}
	}

	Array& operator=(const Array& other) {
		if (this == &other) {
			return *this;
		}
		if (this->data != nullptr) {
			delete[] data;
			data = nullptr;
		}
		width = other.width;
		height = other.height;
		if (width * height > 0) {
			this->data = new T[width * height];
			for (int i = 0; i < width * height; i++) {
				this->data[i] = other.data[i];
			}
		}
		else {
			this->data = nullptr;
		}
		return *this;
	}

	Array(Array&& other) {
		width = other.width;
		height = other.height;
		this->data = other.data;
		other.data = nullptr;
	}
	Array(int w, int h) {
		width = w;
		height = h;
		if (w * h > 0)
			this->data = new T[w * h]();
		else
			this->data = nullptr;
	}

	~Array() {
		if (this->data != nullptr) {
			delete[] data;
			data = nullptr;
		}
	}
	bool in_range(int x, int y) const
	{
		if (x >= this->width || y >= this->height) {
			return false;
		}
		if (x < 0 || y < 0) {
			return false;
		}
		return true;
	}

	T& get(int x, int y) {
		if (this->data == nullptr) {
			throw std::out_of_range("Array is not initialized");
		}
		if (!in_range(x, y)) {
			throw std::out_of_range("Index out of range");
		}
		return this->data[x + y * this->width];
	}
	const T& get(int x, int y) const{
		if (this->data == nullptr) {
			throw std::out_of_range("Array is not initialized");
		}
		if (!in_range(x, y)) {
			throw std::out_of_range("Index out of range");
		}
		return this->data[x + y * this->width];
	}
	int get_width() const {
		return this->width;
	}

	int get_height() const {
		return this->height;
	}
	void fill(T value) {
		for (int i = 0; i < width * height; i++) {
			this->data[i] = value;
		}
	}

};
