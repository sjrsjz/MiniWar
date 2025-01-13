#pragma once
#include <iostream>

template<class T>
class Array {
private:
	int m_width;
	int m_height;
	T* m_data;
public:

	inline T& operator()(int x, int y) {
		return get(x, y);
	}

	inline const T& operator()(int x, int y) const {
		return get(x, y);
	}
	Array() {
		m_width = 0;
		m_height = 0;
		this->m_data = nullptr;
	}

	Array(const Array& other) {
		m_width = other.m_width;
		m_height = other.m_height;
		if (m_width * m_height > 0) {
			this->m_data = new T[m_width * m_height];
			for (int i = 0; i < m_width * m_height; i++) {
				this->m_data[i] = other.m_data[i];
			}
		}
		else {
			this->m_data = nullptr;
		}
	}

	Array& operator=(const Array& other) {
		if (this == &other) {
			return *this;
		}
		if (this->m_data != nullptr) {
			delete[] m_data;
			m_data = nullptr;
		}
		m_width = other.m_width;
		m_height = other.m_height;
		if (m_width * m_height > 0) {
			this->m_data = new T[m_width * m_height];
			for (int i = 0; i < m_width * m_height; i++) {
				this->m_data[i] = other.m_data[i];
			}
		}
		else {
			this->m_data = nullptr;
		}
		return *this;
	}

	Array(Array&& other) {
		m_width = other.m_width;
		m_height = other.m_height;
		this->m_data = other.m_data;
		other.m_data = nullptr;
	}
	Array(int w, int h) {
		m_width = w;
		m_height = h;
		if (w * h > 0)
			this->m_data = new T[w * h]();
		else
			this->m_data = nullptr;
	}

	~Array() {
		if (this->m_data != nullptr) {
			delete[] m_data;
			m_data = nullptr;
		}
	}
	bool in_range(int x, int y) const
	{
		if (x >= this->m_width || y >= this->m_height) {
			return false;
		}
		if (x < 0 || y < 0) {
			return false;
		}
		return true;
	}

	T& get(int x, int y) {
		if (this->m_data == nullptr) {
			throw std::out_of_range("Array is not initialized");
		}
		if (!in_range(x, y)) {
			throw std::out_of_range("Index out of range");
		}
		return this->m_data[x + y * this->m_width];
	}
	const T& get(int x, int y) const{
		if (this->m_data == nullptr) {
			throw std::out_of_range("Array is not initialized");
		}
		if (!in_range(x, y)) {
			throw std::out_of_range("Index out of range");
		}
		return this->m_data[x + y * this->m_width];
	}
	int width() const {
		return this->m_width;
	}

	int height() const {
		return this->m_height;
	}
	void fill(T value) {
		for (int i = 0; i < m_width * m_height; i++) {
			this->m_data[i] = value;
		}
	}

};
