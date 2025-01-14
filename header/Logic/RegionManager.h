#pragma once
#include "../../header/utils/Array.h"
#include "../../header/Logic/Region.h"
#include "../../header/utils/Config.h"
#include  "../../header/Logic/Player.h"
#include "../../header/utils/GlobalTimer.h"
#include <vector>
#include <queue>
#include <mutex>
#include <cmath>
#include <random>
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
public:
	RegionManager();
	RegionManager(int width, int height);
	~RegionManager();

	static RegionManager& instance_of();
	Array2D<Region>& regions();
	Region& region(int x, int y);
	Weapon& get_weapon(int id);
	Player& get_player();
	int map_width();
	int map_height();

	std::vector<MovingArmy> get_moving_army_position();
	std::vector<MovingMissle> get_moving_missle_position();
	int calculate_region_amount(int player_id);

	void calculate_delta_resources(std::vector<double>& delta_resource, double delta_t, int player_id);
	void calculate_steady_cost_resources(std::vector<double>& steady_cost_resource, int player_id);

	void init(int width, int height);
	void update(GlobalTimer& timer);
	double move_army(Point start, Point end, int amount, int army_level);
	void attack_region_missle(int weapon_id, int level, Point start, Point end, double time);
	void attack_region_army(Point start, Point end, int amount);
	const Array2D<int>& neighbour_regions() {
		return m_neighbour_regions;
	}

private:
	int m_width;
	int m_height;
	Array2D<Region> m_regions;
	Array2D<int> m_neighbour_regions; // 邻接区域
	Player m_player;
	std::vector<Weapon> m_weapons;                 // 0: CM, 1: MRBM, 2: ICBM
	std::vector<MovingArmy> m_moving_armies;
	std::vector<MovingMissle> m_moving_missles;
	double m_current_time{};

	std::mutex army_mutex;
	std::mutex missle_mutex;

	void clear_building(Region& region);
	std::vector<Region*> get_damaged_regions(Point position, double range);


	double calculate_path(Point start, Point end, std::vector<std::tuple<int, int>>& path);
	int calculate_neigbour_region(int x, int y); // 计算邻接区域位置
};