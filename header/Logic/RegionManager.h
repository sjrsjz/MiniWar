#pragma once
#include "../utils/Array.h"
#include "region.h"
#include "Player.h"
#include <vector>

class RegionManager {
private:
	Array<region> regions;
	std::vector<Player> players;
	void clear_building();
public:
	void update();
	void attack_region();
};