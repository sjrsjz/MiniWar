#pragma once
#include "../../header/utils/Array.h"
#include "../../header/Logic/Region.h"
#include "../../header/utils/Config.h"
#include  "../../header/Logic/Player.h"
#include <vector>
#include <queue>

struct MovingArmy {
	int amount{};
	double time{};
	std::vector<std::tuple<int, int>> path{};
	bool operator<(const MovingArmy& rhs) const {
		return time > rhs.time;
	}
};

struct MovingMissle {
	int damage{};
	double time{};
	std::tuple<int, int> start_point{};
	std::tuple<int, int> end_point{};
	bool operator<(const MovingMissle& rhs) const {
		return time > rhs.time;
	}
};

class RegionManager {
private:
	int width;
	int height;
	Array<Region> regions;
	Player player;
	std::vector<Weapon> weapons;// 0: CM, 1: MRBM, 2: ICBM
	std::priority_queue<MovingArmy> moving_armies;
	std::priority_queue<MovingMissle> moving_missles;
	double current_time{};
	
	void clear_building(Region& region);
public:
	inline RegionManager() {
		// default constructor


	}
	RegionManager(int width, int height);
	~RegionManager();
	Weapon& get_weapon(int id);
	int get_map_width();
	int get_map_height();
	Player& get_player();
	void update();
	void move_army(int amount, double time, std::vector<std::tuple<int, int>>& path);
	void attack_region_missle(int weapon_id, Point start, Point end, double time, int damage);
	void attack_region_army(Point start, Point end, int amount);
	Array<Region>& get_regions();
	Region& get_region(int x, int y);
	static RegionManager& getInstance();
};
