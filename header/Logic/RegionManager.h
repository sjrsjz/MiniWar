#pragma once
#include "../utils/Array.h"
#include "Region.h"
#include <vector>

class Player;

class RegionManager {
private:
	Array<Region> regions;
	std::vector<Player> players;
	void clear_building();
public:
	RegionManager();
	~RegionManager();
	void update();
	void attack_region();
	void owner_alter();
	Array<Region> get_regions();
	Region * get_region(int x, int y);
};
