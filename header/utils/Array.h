#pragma once
#include <iostream>
#include <vector>
#include <numeric> // for std::accumulate
#include <functional> // for std::multiplies
template<class T>
class Array2D {
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
	Array2D() {
		m_width = 0;
		m_height = 0;
		this->m_data = nullptr;
	}

	Array2D(const Array2D& other) {
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

	Array2D& operator=(const Array2D& other) {
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

	Array2D(Array2D&& other) {
		m_width = other.m_width;
		m_height = other.m_height;
		this->m_data = other.m_data;
		other.m_data = nullptr;
	}
	Array2D(int w, int h) {
		m_width = w;
		m_height = h;
		if (w * h > 0)
			this->m_data = new T[w * h]();
		else
			this->m_data = nullptr;
	}

	~Array2D() {
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




template<class T>
class ArrayND {
private:
	std::vector<int> m_dimensions;
	T* m_data;

	// Helper function to calculate the 1D index from N-dimensional indices
	int calculate_linear_index(const std::vector<int>& indices) const {
		if (indices.size() != m_dimensions.size()) {
			throw std::invalid_argument("Number of indices does not match the number of dimensions");
		}
		int linear_index = 0;
		int stride = 1;
		for (size_t i = 0; i < m_dimensions.size(); ++i) {
			if (indices[i] < 0 || indices[i] >= m_dimensions[i]) {
				throw std::out_of_range("Index out of range");
			}
			linear_index += indices[i] * stride;
			stride *= m_dimensions[i];
		}
		return linear_index;
	}

public:
	// Default constructor
	ArrayND() : m_data(nullptr) {}

	// Constructor with dimensions
	ArrayND(const std::vector<int>& dimensions) : m_dimensions(dimensions) {
		if (std::any_of(m_dimensions.begin(), m_dimensions.end(), [](int dim) { return dim <= 0; })) {
			throw std::invalid_argument("Dimensions must be positive");
		}
		size_t total_size = std::accumulate(m_dimensions.begin(), m_dimensions.end(), 1, std::multiplies<int>());
		if (total_size > 0) {
			m_data = new T[total_size]();
		}
		else {
			m_data = nullptr;
		}
	}

	template<typename... Args>
	ArrayND(Args... args) : m_dimensions({ static_cast<int>(args)... }) {
		if (std::any_of(m_dimensions.begin(), m_dimensions.end(), [](int dim) { return dim <= 0; })) {
			throw std::invalid_argument("Dimensions must be positive");
		}
		size_t total_size = std::accumulate(m_dimensions.begin(), m_dimensions.end(), 1, std::multiplies<int>());
		if (total_size > 0) {
			m_data = new T[total_size]();
		}
		else {
			m_data = nullptr;
		}
	}

	// Copy constructor
	ArrayND(const ArrayND& other) : m_dimensions(other.m_dimensions) {
		size_t total_size = std::accumulate(m_dimensions.begin(), m_dimensions.end(), 1, std::multiplies<int>());
		if (total_size > 0) {
			m_data = new T[total_size];
			for (size_t i = 0; i < total_size; ++i) {
				m_data[i] = other.m_data[i];
			}
		}
		else {
			m_data = nullptr;
		}
	}

	// Copy assignment operator
	ArrayND& operator=(const ArrayND& other) {
		if (this == &other) {
			return *this;
		}
		size_t total_size = std::accumulate(other.m_dimensions.begin(), other.m_dimensions.end(), 1, std::multiplies<int>());

		if (m_data != nullptr) {
			delete[] m_data;
			m_data = nullptr;
		}
		m_dimensions = other.m_dimensions;

		if (total_size > 0) {
			m_data = new T[total_size];
			for (size_t i = 0; i < total_size; ++i) {
				m_data[i] = other.m_data[i];
			}
		}
		return *this;
	}

	// Move constructor
	ArrayND(ArrayND&& other) noexcept : m_dimensions(std::move(other.m_dimensions)), m_data(other.m_data) {
		other.m_data = nullptr;
	}

	// Move assignment operator
	ArrayND& operator=(ArrayND&& other) noexcept {
		if (this == &other) {
			return *this;
		}
		if (m_data != nullptr) {
			delete[] m_data;
			m_data = nullptr;
		}
		m_dimensions = std::move(other.m_dimensions);
		m_data = other.m_data;
		other.m_data = nullptr;
		return *this;
	}

	// Destructor
	~ArrayND() {
		if (m_data != nullptr) {
			delete[] m_data;
			m_data = nullptr;
		}
	}

	// Check if indices are in range
	bool in_range(const std::vector<int>& indices) const {
		if (indices.size() != m_dimensions.size()) {
			return false;
		}
		for (size_t i = 0; i < m_dimensions.size(); ++i) {
			if (indices[i] < 0 || indices[i] >= m_dimensions[i]) {
				return false;
			}
		}
		return true;
	}

	// Get element at specified indices (variadic template)
	template<typename... Args>
	T& get(Args... args) {
		static_assert(sizeof...(Args) == m_dimensions.size(), "Number of arguments must match the number of dimensions");
		std::vector<int> indices = { static_cast<int>(args)... };
		if (m_data == nullptr) {
			throw std::out_of_range("Array is not initialized");
		}
		if (!in_range(indices)) {
			throw std::out_of_range("Index out of range");
		}
		return m_data[calculate_linear_index(indices)];
	}

	// Get element at specified indices (const version, variadic template)
	template<typename... Args>
	const T& get(Args... args) const {
		static_assert(sizeof...(Args) == m_dimensions.size(), "Number of arguments must match the number of dimensions");
		std::vector<int> indices = { static_cast<int>(args)... };
		if (m_data == nullptr) {
			throw std::out_of_range("Array is not initialized");
		}
		if (!in_range(indices)) {
			throw std::out_of_range("Index out of range");
		}
		return m_data[calculate_linear_index(indices)];
	}

	// Access operator (variadic template)
	template<typename... Args>
	T& operator()(Args... args) {
		return get(args...);
	}

	// Access operator (const version, variadic template)
	template<typename... Args>
	const T& operator()(Args... args) const {
		return get(args...);
	}

	// Get the dimensions of the array
	const std::vector<int>& dimensions() const {
		return m_dimensions;
	}

	// Fill the array with a specific value
	void fill(T value) {
		size_t total_size = std::accumulate(m_dimensions.begin(), m_dimensions.end(), 1, std::multiplies<int>());
		for (size_t i = 0; i < total_size; ++i) {
			m_data[i] = value;
		}
	}
};