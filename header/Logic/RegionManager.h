#pragma once
#include "../../header/utils/Array.h"
#include "../../header/Logic/Region.h"
#include <vector>

struct MovingArmy;
struct MovingMissle;

class Player;

class RegionManager {
private:
	Array<Region> regions;
	std::vector<Player> players;
	std::vector<Weapon> weapons;// 0: CM, 1: MRBM, 2: ICBM
	std::vector<MovingArmy> moving_armies;
	std::vector<MovingMissle> moving_missles;
	void clear_building();
public:
	RegionManager(int width, int height, int player_amount);
	~RegionManager();
	Weapon& get_weapon(int id);
	void update();
	void attack_region(int weapon_id, Point start, Point end, double time, int damage);
	void owner_alter();
	Array<Region> get_regions();
	Region& get_region(int x, int y);
};
