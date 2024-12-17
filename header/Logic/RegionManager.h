#pragma once
#include "../utils/Array.h"
#include "Region.h"
#include "Player.h"
#include <vector>

class RegionManager {
private:
	Array<Region> regions;
	std::vector<Player> players;
	void clear_building();
public:
	void update();
	void attack_region();
};
