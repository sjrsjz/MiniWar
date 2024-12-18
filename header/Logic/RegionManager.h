#pragma once
#include "../utils/Array.h"
#include "region.h"
#include <vector>

class Player;
class RegionManager {
private:
	Array<region> regions;
	std::vector<Player> players;
	void clear_building();
public:
	void update();
	void attack_region();
};