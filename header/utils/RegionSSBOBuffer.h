#pragma once
#include "../passes/SSBO.h"
#include "RegionData.h"
// SSBO缓冲区，用于存储区域数据并传递给地图着色器
class RegionSSBOBuffer
{
	RegionData* m_regions;
	SSBO m_ssbo;
	int m_map_width, m_map_height;

public:
	RegionSSBOBuffer() : m_regions(nullptr), m_map_width(0), m_map_height(0) {}
	~RegionSSBOBuffer() {
	}

	void create(int width, int height) {
		m_regions = new RegionData[width * height];
		m_map_width = width;
		m_map_height = height;
	}

	void release() {
		if (m_regions) {
			delete[] m_regions;
			m_regions = nullptr;
		}
	}

	void set_region(int x, int y, RegionData region) {
		if (x < 0 || x >= m_map_width || y < 0 || y >= m_map_height)
			throw std::out_of_range("RegionSSBOBuffer::setRegion out of range");
		m_regions[y * m_map_width + x] = region;
	}
	void create_ssbo() {
		m_ssbo.set_binding_point_index(0);
		m_ssbo = SSBO(m_map_width * m_map_height * sizeof(RegionData), GL_DYNAMIC_DRAW);
	}

	void update() {
		m_ssbo.update_data(m_regions, m_map_width * m_map_height * sizeof(RegionData));
	}

	void bind(GLuint index) {
		m_ssbo.bind(index);
	}

	void unbind() {
		m_ssbo.unbind_ssbo();
	}

	int width() const {
		return m_map_width;
	}

	int height() const {
		return m_map_height;
	}

	void* regions() const {
		return m_regions;
	}

	RegionData& region(int x, int y) {
		if (x < 0 || x >= m_map_width || y < 0 || y >= m_map_height)
			throw std::out_of_range("RegionSSBOBuffer::getRegion out of range");
		return m_regions[y * m_map_width + x];
	}

	const RegionData& region(int x, int y) const {
		if (x < 0 || x >= m_map_width || y < 0 || y >= m_map_height)
			throw std::out_of_range("RegionSSBOBuffer::getRegion out of range");
		return m_regions[y * m_map_width + x];
	}

	// 迭代器，用于遍历区域数据，为 {RegionData&, x, y} 的元组

	class iterator {
		RegionData* ptr;
		int x, y;
		int width;
	public:
		iterator(RegionData* ptr, int x, int y, int width) : ptr(ptr), x(x), y(y), width(width) {}
		iterator& operator++() {
			++x;
			if (x == width) {
				x = 0;
				++y;
			}
			return *this;
		}
		bool operator!=(const iterator& other) const {
			return x != other.x || y != other.y;
		}
		std::tuple<RegionData&, int, int> operator*() {
			return { ptr[y * width + x], x, y };
		}
	};

	iterator begin() {
		return iterator(m_regions, 0, 0, m_map_width);
	}

	iterator end() {
		return iterator(m_regions, 0, m_map_height, m_map_width);
	}


};