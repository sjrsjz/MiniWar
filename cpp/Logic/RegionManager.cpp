#include "../../header/Logic/RegionManager.h"
#include "../../header/debug.h"
#include "../../header/Logic/Player.h"
#include "../../header/Logic/Resource.h"
#include "../../header/Logic/GameEffect.h"

#include <cmath>
#include <vector>
#include <random>
#include <mutex>

std::mutex army_mutex;
std::mutex missle_mutex;

RegionManager::RegionManager() {
}

RegionManager::RegionManager(int width, int height){
	set(width, height);
}

RegionManager::~RegionManager() {

}

std::vector<Point> astar(const Array<int>& grid, Point start, Point end) {
	int width = grid.width();
	int height = grid.height();
	Array<bool> closed(width, height);
	auto cmp = [](const std::pair<Point, double>& a, const std::pair<Point, double>& b) {
		return a.second > b.second;
		};
	std::priority_queue<std::pair<Point, double>, std::vector<std::pair<Point, double>>, decltype(cmp)> open(cmp);

	// 记录每个节点的g值
	Array<double> g_scores(width, height);
	g_scores.fill(std::numeric_limits<double>::infinity());
	g_scores(std::floor(start.x), std::floor(start.y)) = 0;

	enum direction { E, W, S, N };
	Array<direction> directions(width, height);

	open.push({ start, 0.0 });

	const int dx[4] = { 1, -1, 0, 0 };
	const int dy[4] = { 0, 0, 1, -1 };

	while (!open.empty()) {
		Point current = open.top().first;
		open.pop();

		int x = std::floor(current.x);
		int y = std::floor(current.y);

		if (current == end) {
			std::vector<Point> path;
			Point position = end;
			while (position != start) {
				path.push_back(position);
				direction dir = directions(std::floor(position.x), std::floor(position.y));
				position.x -= dx[dir]; // 反向回溯
				position.y -= dy[dir];
			}
			path.push_back(start);
			std::reverse(path.begin(), path.end());
			return path;
		}

		if (closed(x, y)) continue;
		closed(x, y) = true;

		for (int i = 0; i < 4; i++) {
			int nx = x + dx[i];
			int ny = y + dy[i];
			if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
			if (closed(nx, ny) || grid(nx, ny) == 0) continue;

			Point next(nx, ny);
			double tentative_g = g_scores(x, y) + current.distance(next);

			if (tentative_g < g_scores(nx, ny)) {
				g_scores(nx, ny) = tentative_g;
				directions(nx, ny) = static_cast<direction>(i);
				double h = next.distance(end);
				open.push({ next, tentative_g + h });
			}
		}
	}
	return {};
}



double RegionManager::calculate_distance(Point start, Point end, std::vector<std::tuple<int, int>>& path) {
	DEBUGOUTPUT("RegionManager::calculate_distance() called");
	int start_x = std::floor(start.x);
	int start_y = std::floor(start.y);
	int end_x = std::floor(end.x);
	int end_y = std::floor(end.y);




	int id = region(start_x, start_y).get_owner();

	double distance = 0.f;
	Array<int> player_region_matrix(m_regions.width(), m_regions.height());
	for (int i = 0; i < m_regions.width(); i++) {
		for (int j = 0; j < m_regions.height(); j++) {
			if (m_regions(i, j).get_owner() == id || m_regions(i, j).get_owner() == -1 || (i == end_x && j == end_y)) {
				player_region_matrix(i, j) = 1;
			}
		}
	}
	std::vector<Point> path_points = astar(player_region_matrix, start, end);

	if (path_points.empty()) {
		return -1.f;
	}

	for (int i = 0; i < path_points.size(); i++) {
		path.push_back(std::make_tuple(path_points[i].x, path_points[i].y));
		if (i == 0) continue;
		distance += path_points[i].distance(path_points[i - 1]);
	}

	return distance;
}
std::vector<Region*> RegionManager::get_damaged_regions(Point position, double range) {
	double start_x = position.x;
	double start_y = position.y;
	std::vector<Region*> result;
	int range_R = std::ceil(range);
	for (int i = -range_R; i <= range_R; i++) {
		for (int j = -range_R; j <= range_R; j++) {
			if (m_regions.in_range(start_x + i, start_y + j) && std::sqrt(i * i + j * j) <= range) {
					result.push_back(&region(start_x + i, start_y + j));
			}
		}
	}
	return result;
}

Weapon& RegionManager::get_weapon(int id) {
	return m_weapons[id];
}

int RegionManager::map_width() {
	return m_width;
}

int RegionManager::map_height() {
	return m_height;
}

Player& RegionManager::get_player() {
	return m_player;
}

std::vector<MovingArmy> RegionManager::get_moving_army_position() {
	std::vector<MovingArmy> copy;
	{
		std::lock_guard<std::mutex> lock(army_mutex);
		copy = m_moving_armies;
	}

	return copy;

}

std::vector<MovingMissle> RegionManager::get_moving_missle_position() {
	std::vector<MovingMissle> copy;
	{
		std::lock_guard<std::mutex> lock(missle_mutex);
		copy = m_moving_missles;
	}
	return copy;
}

void RegionManager::set(int width, int height) {
	DEBUGOUTPUT("RegionManager::set() called");
	this->m_regions.~Array();
	this->m_regions = Array<Region>(width, height);
	this->m_width = width;
	this->m_height = height;
	this->m_player.create();
	m_weapons.clear();
	m_moving_armies = {};
	m_moving_missles = {};

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			m_regions(x, y) = Region(x, y);
		}
	}
	for (int i = 0; i <= 2; i++) {
		Weapon weapon(i);
		m_weapons.push_back(weapon);
	}

}

double RegionManager::move_army(Point start, Point end, int amount, int army_level) {
	DEBUGOUTPUT("RegionManager::move_army() called");
	std::vector<std::tuple<int, int>> path;

	double distance = calculate_distance(start, end, path);
	if (distance == -1.f) {
		DEBUGOUTPUT("RegionManager::move_army() can not find a path");
		throw std::invalid_argument(u8"无法移动军队到指定位置");
	}

	int start_x = std::floor(start.x);
	int start_y = std::floor(start.y);
	int end_x = std::floor(end.x);
	int end_y = std::floor(end.y);

	Region& start_region = region(start_x, start_y);
	Region& end_region = region(end_x, end_y);

	Config& configer = Config::instance_of();
	double speed = configer.get_army_parameter().speed[army_level - 1];

	double time = distance / speed;

	start_region.reduce_army_amount(amount);

	MovingArmy army;
	army.owner_id = start_region.get_owner();
	army.amount = amount;
	army.time = time;
	army.reach_time = m_current_time + time;
	army.path = path;
	army.current_pos = std::make_tuple(start_x + 0.5, end_x + 0.5);
	army_mutex.lock();
	m_moving_armies.push_back(army);
	army_mutex.unlock();

	//count time
	//if time is up, move
	return time;
}

void RegionManager::attack_region_missle(int weapon_id, int level, Point start, Point end, double time) {
	Region& start_region = region(start.x, start.y);
	Region& end_region = region(end.x, end.y);
	MovingMissle missle;
	missle.weapon_id = weapon_id;
	missle.weapon_level = level;
	missle.owner_id = start_region.get_owner();
	missle.time = time;
	missle.reach_time = m_current_time + time;
	missle.start_point = std::make_tuple(std::floor(start.x), std::floor(start.y));
	missle.end_point = std::make_tuple(std::floor(end.x), std::floor(end.y));
	missle.current_pos = std::make_tuple(std::floor(start.x) + 0.5, std::floor(end.y) + 0.5, 0);
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(4000, 5000);
	std::uniform_int_distribution<>	dis2(0, 2000);
	missle.h = dis(gen);
	missle.M = dis2(gen);

	//DEBUGOUTPUT("RegionManager::attack_region_missle() Lock missle mutex");
	missle_mutex.lock();
	m_moving_missles.push_back(missle);
	missle_mutex.unlock();
	//DEBUGOUTPUT("RegionManager::attack_region_missle() Unlock missle mutex");
}

void RegionManager::attack_region_army(Point start, Point end, int amount) {
	std::vector<std::tuple<int, int>> path;
	double distance = calculate_distance(start, end, path);

	Region& start_region = region(start.x, start.y);
	Region& end_region = region(end.x, end.y);

	double time = distance / start_region.get_army().get_speed();
	DEBUGOUTPUT("moving cost time ", time);
	MovingArmy army;
	army.amount = amount;
	army.path = path;
	army.time = m_current_time + time;
	army.owner_id = start_region.get_owner();

	start_region.reduce_army_amount(amount);
	army_mutex.lock();
	m_moving_armies.push_back(army);
	army_mutex.unlock();
	
	//count time
	//if time is up, attack
	//if time is up, remove army
	
	/*int rest_army = amount - end_region.getArmy().getForce();
	end_region.setOwner(start_region.getOwner());
	end_region.getArmy().removeArmy(end_region.getArmy().getForce());
	end_region.getArmy().addArmy(rest_army);
	end_region.getWeapons().clear();
	clear_building(end_region);*/
}

Array<Region>& RegionManager::regions() {
	return m_regions;
}

Region& RegionManager::region(int x, int y) {
	return m_regions(x, y);
}

static RegionManager instance;
RegionManager& RegionManager::instance_of() {
	return instance;
}

void RegionManager::clear_building(Region& region) {
	region.get_building().remove();
}

void push_game_effects(GameEffect effect);

void RegionManager::update(GlobalTimer& timer) {
	m_current_time = timer.get_running_time();

	//DEBUGOUTPUT("RegionManager::update() called", moving_armies.size());
	{
		std::lock_guard<std::mutex> lock(army_mutex);
		std::vector<MovingArmy> temp_armies;

		for (auto& army : m_moving_armies){
			//DEBUGOUTPUT("Moving army: ", army.amount);
			if(army.reach_time > m_current_time) {
				temp_armies.push_back(army);
				continue;
			}
			if (army.path.size() < 2) continue;
			Region& end_region = region(std::get<0>(army.path.back()), std::get<1>(army.path.back()));
			Region& start_region = region(std::get<0>(army.path.front()), std::get<1>(army.path.front()));
			if (end_region.get_owner() == start_region.get_owner())
			{
				end_region.add_army_amount(army.amount);
			}
			else {
				int rest_army = army.amount - end_region.get_army().get_force();
				if (rest_army < 0) {
					end_region.get_army().set_amount(-rest_army);
				}
				else {
					end_region.set_owner(start_region.get_owner());
					end_region.get_army().set_amount(rest_army);
					end_region.get_weapons().clear();
					clear_building(end_region);
				}
			}
		}
		m_moving_armies.swap(temp_armies);
	}

	//DEBUGOUTPUT("RegionManager::update() Lock missle mutex", "1st");

	{
		std::lock_guard<std::mutex> lock(missle_mutex);
		std::vector<MovingMissle> swap_missles;
		for (auto& missle : m_moving_missles) {
			if (missle.reach_time > m_current_time) {
				swap_missles.push_back(missle);
				continue;
			}
			Region& end_region = region(std::get<0>(missle.end_point), std::get<1>(missle.end_point));
			
			Weapon weapon = get_weapon(missle.weapon_id);
			int damage = weapon.get_damage(missle.weapon_level);
			double damage_range = weapon.get_damage_range(missle.weapon_level);
			
			std::vector<Region*> damaged_regions = get_damaged_regions(end_region.get_center_position(), damage_range);
			
			for (auto& region : damaged_regions)
			{
				int rest_hp = region->get_HP() - damage;
				if (rest_hp <= 0) {
					region->set_owner(-1);
					std::random_device rd;
					std::mt19937 gen(rd());
					std::uniform_int_distribution<> dis(200, 300);
					region->set_HP(dis(gen));
					region->get_weapons().clear();
					region->get_army().set_amount(0);
					clear_building(*region);
				}
				else {
					region->set_HP(rest_hp);
				}
			}
			push_game_effects(GameEffect::GAME_EFFECT_PLAY_NUCLEAR_EXPLOSION);
		}
		m_moving_missles.swap(swap_missles);
	}
	//DEBUGOUTPUT("RegionManager::update() Unlock missle mutex", "1st");

	{
		std::lock_guard<std::mutex> lock(army_mutex);
		for (auto& army : m_moving_armies) {
			//update current position
			int side_num = army.path.size() - 1;
			int current_side = army.time == 0 ? 0 : (1 + (m_current_time - army.reach_time) / army.time) * side_num;

			current_side = current_side > side_num ? side_num : current_side;

			double mix = (1 + (m_current_time - army.reach_time) / army.time) * side_num - current_side;

			int current_x_L = std::get<0>(army.path[current_side]);
			int current_y_L = std::get<1>(army.path[current_side]);

			current_side = current_side + 1 >= army.path.size() ? current_side : current_side + 1;

			int current_x_R = std::get<0>(army.path[current_side]);
			int current_y_R = std::get<1>(army.path[current_side]);


			double center_x_L = region(current_x_L, current_y_L).get_center_position().x;
			double center_y_L = region(current_x_L, current_y_L).get_center_position().y;

			double center_x_R = region(current_x_R, current_y_R).get_center_position().x;
			double center_y_R = region(current_x_R, current_y_R).get_center_position().y;

			double mix_x = center_x_L * (1 - mix) + center_x_R * mix;
			double mix_y = center_y_L * (1 - mix) + center_y_R * mix;

			army.current_pos = std::make_tuple(mix_x, mix_y);
		}
	}
	//DEBUGOUTPUT("RegionManager::update() Lock missle mutex", "2nd");
	{
		std::lock_guard<std::mutex> lock(missle_mutex);
		for (auto& missle : m_moving_missles) {
			//update current postion
			double mix = fmin(1, (1 + (m_current_time - missle.reach_time) / missle.time));
			//DEBUGOUTPUT("mix: ", mix);
			auto start = missle.start_point;
			auto end = missle.end_point;
			std::tuple<int, int> middle = std::make_tuple((std::get<0>(start) + std::get<0>(end)) / 2, (std::get<1>(start) + std::get<1>(end)) / 2);
			auto [middleX0, middleY0] = middle;
			auto [startX0, startY0] = start;
			auto [endX0, endY0] = end;

			double startX = region(startX0, startY0).get_center_position().x;
			double startY = region(startX0, startY0).get_center_position().y;

			double endX = region(endX0, endY0).get_center_position().x;
			double endY = region(endX0, endY0).get_center_position().y;

			double middleX = region(middleX0, middleY0).get_center_position().x;
			double middleY = region(middleX0, middleY0).get_center_position().y;


			double M = missle.M / 10000.0;

			double P1X = startX - (middleX - startX) * M;
			double P1Y = startY - (middleY - startY) * M;

			double P2X = middleX - (middleX - endX) * M;
			double P2Y = middleY - (middleY - endY) * M;

			double h = missle.h / 1000.0;
			double x = (1 - mix) * (1 - mix) * (1 - mix) * startX + 3 * mix * (1 - mix) * (1 - mix) * P1X + 3 * mix * mix * (1 - mix) * P2X + mix * mix * mix * endX;
			double y = (1 - mix) * (1 - mix) * (1 - mix) * startY + 3 * mix * (1 - mix) * (1 - mix) * P1Y + 3 * mix * mix * (1 - mix) * P2Y + mix * mix * mix * endY;
			double z = (3 * h * (1 - mix) * (1 - mix) * mix + 3 * h * (1 - mix) * mix * mix) * 0.025;
			missle.current_pos = std::make_tuple(x, y, z);
		}

	}
	//DEBUGOUTPUT("RegionManager::update() Unlock missle mutex", "2nd");
}

void RegionManager::calculate_delta_resources(std::vector<double>& delta_resource, double delta_t, int player_id) {
	Config& configer = Config::instance_of();
	for (int i = 0; i < m_width; i++) {
		for (int j = 0; j < m_height; j++) {
			Region& region = this->region(i, j);
			std::string building_name = BuildingTypeToString(region.get_building().get_type());
			if (region.get_owner() == player_id && region.get_building().get_type() != BuildingType::None) {
				int level = region.get_building().get_level() - 1;
				delta_resource[ResourceType::ELECTRICITY] += configer.get_building_setting(building_name).ResourceGeneration[level][ResourceType::ELECTRICITY] * delta_t;
				delta_resource[ResourceType::OIL] += configer.get_building_setting(building_name).ResourceGeneration[level][ResourceType::OIL] * delta_t;
				delta_resource[ResourceType::STEEL] += configer.get_building_setting(building_name).ResourceGeneration[level][ResourceType::STEEL] * delta_t;
				delta_resource[ResourceType::GOLD] += configer.get_building_setting(building_name).ResourceGeneration[level][ResourceType::GOLD] * delta_t;
				delta_resource[ResourceType::LABOR] += configer.get_building_setting(building_name).ResourceGeneration[level][ResourceType::LABOR] * delta_t;

			}
		}
	}
}

void RegionManager::calculate_steady_cost_resources(std::vector<double>& steady_cost_resource, int player_id) {
	Config& configer = Config::instance_of();
	for (int i = 0; i < m_width; i++) {
		for (int j = 0; j < m_height; j++) {
			Region& region = this->region(i, j);
			std::string building_name = BuildingTypeToString(region.get_building().get_type());
			if (region.get_owner() == player_id && region.get_building().get_type() != BuildingType::None) {
				int level = region.get_building().get_level() - 1;

				steady_cost_resource[ResourceType::ELECTRICITY] += configer.get_building_setting(building_name).SteadyCost[level][ResourceType::ELECTRICITY];
				steady_cost_resource[ResourceType::OIL] += configer.get_building_setting(building_name).SteadyCost[level][ResourceType::OIL];
				steady_cost_resource[ResourceType::STEEL] += configer.get_building_setting(building_name).SteadyCost[level][ResourceType::STEEL];
				steady_cost_resource[ResourceType::GOLD] += configer.get_building_setting(building_name).SteadyCost[level][ResourceType::GOLD];
				steady_cost_resource[ResourceType::LABOR] += configer.get_building_setting(building_name).SteadyCost[level][ResourceType::LABOR];

			}
		}
	}
}

int RegionManager::calculate_region_amount(int player_id) {
	int count = 0;
	Config& configer = Config::instance_of();
	for (int i = 0; i < m_width; i++) {
		for (int j = 0; j < m_height; j++) {
			if (region(i, j).get_owner() == player_id) {
				count++;
			}
		}
	}
	return count;
}