#pragma once
#include "../../header/utils/Array.h"
#include "../../header/Logic/Region.h"
#include "../../header/utils/Config.h"
#include  "../../header/Logic/Player.h"
#include "../../header/utils/GlobalTimer.h"
#include <vector>
#include <queue>

struct MovingArmy {
	int owner_id{ -1 };
	int amount{};
	double time{};
	double reach_time{};
	std::vector<std::tuple<int, int>> path{};
	std::tuple<double, double> current_pos{};
	bool operator<(const MovingArmy& rhs) const {
		return reach_time > rhs.reach_time;
	}
};

struct MovingMissle {
	int owner_id{ -1 };
	int weapon_id{};
	int weapon_level{};
	double time{};
	double reach_time{};
	std::tuple<int, int> start_point{};
	std::tuple<int, int> end_point{};
	std::tuple<double, double, double> current_pos{};
	int h;
	int M;
	bool operator<(const MovingMissle& rhs) const {
		return reach_time > rhs.reach_time;
	}
};

class Player;
class RegionManager {
private:
	int m_width;
	int m_height;
	Array<Region> m_regions;
	Player m_player;
	std::vector<Weapon> m_weapons;// 0: CM, 1: MRBM, 2: ICBM
	std::vector<MovingArmy> m_moving_armies;
	std::vector<MovingMissle> m_moving_missles;
	double m_current_time{};
	
	void clear_building(Region& region);
	std::vector<Region*> get_damaged_regions(Point position, double range);
public:
	RegionManager();
	RegionManager(int width, int height);
	~RegionManager();
	Weapon& get_weapon(int id);
	int map_width();
	int map_height();
	Player& get_player();

	std::vector<MovingArmy> get_moving_army_position();
	std::vector<MovingMissle> get_moving_missle_position();

	void set(int width, int height);

	void calculate_delta_resources(std::vector<double>& delta_resource, double delta_t, int player_id);
	void calculate_steady_cost_resources(std::vector<double>& steady_cost_resource, int player_id);
	int calculate_region_amount(int player_id);
	void update(GlobalTimer& timer);

	double move_army(Point start, Point end, int amount, int army_level);
	void attack_region_missle(int weapon_id, int level, Point start, Point end, double time);
	void attack_region_army(Point start, Point end, int amount);

	double calculate_distance(Point start, Point end, std::vector<std::tuple<int, int>>& path);

	Array<Region>& regions();
	Region& region(int x, int y);
	static RegionManager& instance_of();
};
