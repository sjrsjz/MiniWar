#ifndef REGION_SSBO_BUFFER_H
#define REGION_SSBO_BUFFER_H
#include "../passes/SSBO.h"
#include "RegionData.h"
// SSBO缓冲区，用于存储区域数据并传递给地图着色器
class RegionSSBOBuffer {
	RegionData* regions;
	SSBO ssbo;
	int mapWidth, mapHeight;

public:
	RegionSSBOBuffer() : regions(nullptr), mapWidth(0), mapHeight(0) {}
	~RegionSSBOBuffer() {
	}

	void create(int width, int height) {
		regions = new RegionData[width * height];
		mapWidth = width;
		mapHeight = height;
	}

	void release() {
		if (regions) {
			delete[] regions;
			regions = nullptr;
		}
	}

	void setRegion(int x, int y, RegionData region) {
		if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight)
			throw std::out_of_range("RegionSSBOBuffer::setRegion out of range");
		regions[y * mapWidth + x] = region;
	}
	void create_ssbo() {
		ssbo.set_binding_point_index(0);
		ssbo = SSBO(mapWidth * mapHeight * sizeof(RegionData), GL_DYNAMIC_DRAW);
	}

	void update() {
		ssbo.update_data(regions, mapWidth * mapHeight * sizeof(RegionData));
	}

	void bind(GLuint index) {
		ssbo.bind(index);
	}

	void unbind() {
		ssbo.unbind_ssbo();
	}

	int getWidth() const {
		return mapWidth;
	}

	int getHeight() const {
		return mapHeight;
	}

	void* getRegions() const {
		return regions;
	}

	RegionData& getRegion(int x, int y) {
		if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight)
			throw std::out_of_range("RegionSSBOBuffer::getRegion out of range");
		return regions[y * mapWidth + x];
	}

	const RegionData& getRegion(int x, int y) const {
		if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight)
			throw std::out_of_range("RegionSSBOBuffer::getRegion out of range");
		return regions[y * mapWidth + x];
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
		return iterator(regions, 0, 0, mapWidth);
	}

	iterator end() {
		return iterator(regions, 0, mapHeight, mapWidth);
	}


};
#endif