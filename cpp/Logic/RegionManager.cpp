#include "../../header/Logic/RegionManager.h"
#include "../../header/debug.h"
#include "../../header/Logic/Player.h"
#include <vector>
#include <random>


RegionManager::RegionManager() {
}

RegionManager::RegionManager(int width, int height) : width(width), height(height), regions(width, height), moving_missles(), moving_armies(){
	//players = std::vector<Player>(player_amount);

	//players.reserve(player_amount);

	//read configer, initialize Weapon
	DEBUG::DebugOutput("RegionManager::RegionManager() initialized");
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			regions(x, y) = Region(x, y);
		}
	}

	for (int i = 0; i <= 2; i++) {
		Weapon weapon(i);
		weapons.push_back(weapon);
	}
}

RegionManager::~RegionManager() {
}

double RegionManager::calculate_Euclidean_distance(std::tuple<int, int> start, std::tuple<int, int> end) {
	DEBUG::DebugOutput("RegionManager::calculate_Euclidean_distance() called");
	Region& start_region = get_region(std::get<0>(start), std::get<1>(start));
	Region& end_region = get_region(std::get<0>(end), std::get<1>(end));
	Point start_region_position = start_region.getPosition();
	Point end_region_position = end_region.getPosition();
	return sqrt(pow(start_region_position.getX() - end_region_position.getX(), 2) + pow(start_region_position.getY() - end_region_position.getY(), 2));
}

double RegionManager::calculate_distance(Point start, Point end, std::vector<std::tuple<int, int>>& path) {
	DEBUG::DebugOutput("RegionManager::calculate_distance() called");
	int start_x = std::floor(start.getX());
	int start_y = std::floor(start.getY());
	int end_x = std::floor(end.getX());
	int end_y = std::floor(end.getY());

	path.push_back(std::make_tuple(start_x, start_y));//add start point

	int id = get_region(start_x, start_y).getOwner();

	double distance = 0.f;

	Array<Region>& regions = get_regions();
	std::vector<int> player_region_matrix(regions.get_width() * regions.get_height());
	for (int i = 0; i < regions.get_width(); i++) {
		for (int j = 0; j < regions.get_height(); j++) {
			if (regions(i, j).getOwner() == id || regions(i,j).getOwner() == -1 || (i == end_x&&j==end_y)) {
				player_region_matrix[i + j * regions.get_width()] = 1;
			}
			else
			{
				player_region_matrix[i + j * regions.get_width()] = 0;
			}
		}
	}

	int temp_x = start_x;
	int temp_y = start_y;

	static int NOT_AVALIABLE = 1000000;
	static int straight_cost = 10;
	static int diagonal_cost = 14;

	int cost_list[8] = {};//0: left up 1: up 2: right up 3: left 4: right 5: left down 6: down 7: right down

	while (temp_x != end_x && temp_y != end_y)
	{
		path.push_back(std::make_tuple(temp_x, temp_y));

		bool up = true;
		bool down = true;
		bool left = true;
		bool right = true;
		if (temp_x - 1 < 0)
		{
			cost_list[0] = NOT_AVALIABLE;
			cost_list[3] = NOT_AVALIABLE;
			cost_list[5] = NOT_AVALIABLE;
			left = false;
		}
		if (temp_x + 1 > regions.get_width())
		{
			cost_list[2] = NOT_AVALIABLE;
			cost_list[4] = NOT_AVALIABLE;
			cost_list[7] = NOT_AVALIABLE;
			right = false;
		}
		if (temp_y - 1 < 0)
		{
			cost_list[5] = NOT_AVALIABLE;
			cost_list[6] = NOT_AVALIABLE;
			cost_list[7] = NOT_AVALIABLE;
			down = false;
		}
		if (temp_y + 1 > regions.get_height())
		{
			cost_list[0] = NOT_AVALIABLE;
			cost_list[1] = NOT_AVALIABLE;
			cost_list[2] = NOT_AVALIABLE;
			up = false;
		}
		if (up) {
			if (player_region_matrix[temp_x + (temp_y + 1) * regions.get_width()] == 1) {
				cost_list[0] = straight_cost * (1 + end_x - temp_x + end_y - temp_y);
			}
			else {
				cost_list[0] = NOT_AVALIABLE;
			}
			if (left) {
				if (player_region_matrix[temp_x - 1 + (temp_y + 1) * regions.get_width()] == 1) {
					cost_list[1] = diagonal_cost + straight_cost * (end_x - temp_x + end_y - temp_y);
				}
				else {
					cost_list[1] = NOT_AVALIABLE;
				}
			}
			if (right) {
				if (player_region_matrix[temp_x + 1 + (temp_y + 1) * regions.get_width()] == 1) {
					cost_list[2] = diagonal_cost + straight_cost * (end_x - temp_x + end_y - temp_y);
				}
				else {
					cost_list[2] = NOT_AVALIABLE;
				}
			}
		}
		if (down) {
			if (player_region_matrix[temp_x + (temp_y - 1) * regions.get_width()] == 1) {
				cost_list[5] = straight_cost * (1 + end_x - temp_x + end_y - temp_y);
			}
			else {
				cost_list[5] = NOT_AVALIABLE;
			}
			if (left) {
				if (player_region_matrix[temp_x - 1 + (temp_y - 1) * regions.get_width()] == 1) {
					cost_list[6] = diagonal_cost + straight_cost * (end_x - temp_x + end_y - temp_y);
				}
				else {
					cost_list[6] = NOT_AVALIABLE;
				}
			}
			if (right) {
				if (player_region_matrix[temp_x + 1 + (temp_y - 1) * regions.get_width()] == 1) {
					cost_list[7] = diagonal_cost + straight_cost * (end_x - temp_x + end_y - temp_y);
				}
				else {
					cost_list[7] = NOT_AVALIABLE;
				}
			}
		}
		if (left) {
			if (player_region_matrix[temp_x - 1 + temp_y * regions.get_width()] == 1) {
				cost_list[3] = straight_cost * (1 + end_x - temp_x + end_y - temp_y);
			}
			else {
				cost_list[3] = NOT_AVALIABLE;
			}
		}
		if (right) {
			if (player_region_matrix[temp_x + 1 + temp_y * regions.get_width()] == 1) {
				cost_list[4] = straight_cost * (1 + end_x - temp_x + end_y - temp_y);
			}
			else {
				cost_list[4] = NOT_AVALIABLE;
			}
		}

		int min_cost = NOT_AVALIABLE, min_way = 0;
		for (int i = 0; i < 8; i++) {
			if (cost_list[i] < min_cost) {
				min_cost = cost_list[i];
				min_way = i;
			}
		}
		if (min_cost == NOT_AVALIABLE) {
			return -1.f;
		}

		switch (min_way) {
		case 0:
			temp_y++;
			distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x, temp_y - 1));
			break;
		case 1:
			temp_x--;
			temp_y++;
			distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x + 1, temp_y - 1));
			break;
		case 2:
			temp_x++;
			temp_y++;
			distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x - 1, temp_y - 1));
			break;
		case 3:
			temp_x--;
			distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x + 1, temp_y));
			break;
		case 4:
			temp_x++;
			distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x - 1, temp_y));
			break;
		case 5:
			temp_y--;
			distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x, temp_y + 1));
			break;
		case 6:
			temp_x--;
			temp_y--;
			distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x + 1, temp_y + 1));
			break;
		case 7:
			temp_x++;
			temp_y--;
			distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x - 1, temp_y + 1));
			break;
		}
		player_region_matrix[temp_x + temp_y * regions.get_width()] = 0;
	}

	path.push_back(std::make_tuple(end_x, end_y));// add end point

	return distance;
}

std::vector<Region> RegionManager::get_damaged_regions(Point position, float range) {
	float start_x = std::floor(position.getX()) + 0.5;
	float start_y = std::floor(position.getY()) + 0.5;
	std::vector<Region> result;

	int left_range = std::floor(start_x - range < 0.f ? 0.f : start_x - range);
	int right_range = std::floor(start_x + range > get_map_width() ? get_map_width() : start_x + range);
	int down_range = std::floor(start_y - range < 0.f ? 0.f : start_y - range);
	int up_range = std::floor(start_y + range > get_map_width() ? get_map_width() : start_x + range);

	for (int i = left_range; i <= right_range; i++) {
		for (int j = down_range; j <= up_range; j++) {
			float x = i + 0.5;
			float y = j + 0.5;
			if (range <= sqrt(pow(x - start_x, 2) + pow(y - start_y, 2))) {
				result.push_back(get_region(i , j));
			}
		}
	}

	return result;
}

Weapon& RegionManager::get_weapon(int id) {
	return weapons[id];
}

int RegionManager::get_map_width() {
	return width;
}

int RegionManager::get_map_height() {
	return height;
}

Player& RegionManager::get_player() {
	return player;
}

std::vector<MovingArmy> RegionManager::get_moving_army_position() {
	std::vector<MovingArmy> copied_armies;
	std::priority_queue<MovingArmy> copy = moving_armies;

	while (!copy.empty())
	{
		MovingArmy army = copy.top();
		copied_armies.push_back(army);
		copy.pop();
	}
	return copied_armies;
}

std::vector<MovingMissle> RegionManager::get_moving_missle_position() {
	std::vector<MovingMissle> copied_missles;
	std::priority_queue<MovingMissle> copy = moving_missles;

	while (!copy.empty())
	{
		MovingMissle missle = copy.top();
		copied_missles.push_back(missle);
		copy.pop();
	}
	return copied_missles;
}

void RegionManager::set(int width, int height) {
	DEBUG::DebugOutput("RegionManager::set() called");
	this->regions.~Array();
	this->regions = Array<Region>(width, height);
	this->width = width;
	this->height = height;

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			regions(x, y) = Region(x, y);
		}
	}
	for (int i = 0; i <= 2; i++) {
		Weapon weapon(i);
		weapons.push_back(weapon);
	}
}

//void RegionManager::move_army(int amount, double time, std::vector<std::tuple<int, int>>& path) {
//	MovingArmy army;
//	army.amount = amount;
//	army.time = time;
//	Region end_region = get_region(std::get<0>(path.back()), std::get<1>(path.back()));
//
//	army.path = path;
//	moving_armies.push(army);
//	//count time
//	//if time is up, move
//	end_region.addArmy(amount);
//}

double RegionManager::move_army(Point start, Point end, int amount, int army_level) {
	DEBUG::DebugOutput("RegionManager::move_army() called");
	std::vector<std::tuple<int, int>> path;

	double distance = calculate_distance(start, end, path);
	if (distance == -1.f) {
		DEBUG::DebugOutput("RegionManager::move_army() throws");
		throw "Can't find a path";
	}

	int start_x = std::floor(start.getX());
	int start_y = std::floor(start.getY());
	int end_x = std::floor(end.getX());
	int end_y = std::floor(end.getY());

	Region& start_region = get_region(start_x, start_y);
	Region& end_region = get_region(end_x, end_y);

	Config& configer = Config::getInstance();
	double speed = configer.getConfig({ "Army","speed" }).template get<std::vector<double>>()[army_level-1];

	double time = distance / speed;

	start_region.removeArmy(amount);

	MovingArmy army;
	army.owner_id = start_region.getOwner();
	army.amount = amount;
	army.time = time;
	army.reach_time = current_time + time;
	army.path = path;
	army.current_pos = std::make_tuple(start_x + 0.5, end_x + 0.5);
	moving_armies.push(army);

	//count time
	//if time is up, move
	return time;
}

void RegionManager::attack_region_missle(int weapon_id, int level, Point start, Point end, double time) {
	Region& start_region = get_region(start.getX(), start.getY());
	Region& end_region = get_region(end.getX(), end.getY());
	MovingMissle missle;
	missle.weapon_id = weapon_id;
	missle.weapon_level = level;
	missle.owner_id = start_region.getOwner();
	missle.time = time;
	missle.reach_time = current_time + time;
	missle.start_point = std::make_tuple(std::floor(start.getX()), std::floor(start.getY()));
	missle.end_point = std::make_tuple(std::floor(end.getX()), std::floor(end.getY()));
	missle.current_pos = std::make_tuple(std::floor(start.getX()) + 0.5, std::floor(end.getY()) + 0.5);
	moving_missles.push(missle);
	//count time
	//if time is up, attack
	//if time is up, remove missle

	//int rest_hp = end_region.getHp() - damage;
	//if (rest_hp <= 0) {
	//	end_region.setOwner(-1);
	//	end_region.setHp(0);
	//	end_region.getWeapons().clear();
	//	clear_building(end_region);
	//}
	//else {
	//	end_region.setHp(rest_hp);
	//}
}

void RegionManager::attack_region_army(Point start, Point end, int amount) {
	std::vector<std::tuple<int, int>> path;
	double distance = calculate_distance(start, end, path);

	Region& start_region = get_region(start.getX(), start.getY());
	Region& end_region = get_region(end.getX(), end.getY());

	double time = distance / start_region.getArmy().getSpeed();

	MovingArmy army;
	army.amount = amount;
	army.path = path;
	army.time = current_time + time;
	army.owner_id = start_region.getOwner();

	start_region.removeArmy(amount);
	moving_armies.push(army);
	
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

Array<Region>& RegionManager::get_regions() {
	return regions;
}

Region& RegionManager::get_region(int x, int y) {
	return regions(x, y);
}

RegionManager& RegionManager::getInstance() {
	static RegionManager instance;
	return instance;
}

void RegionManager::clear_building(Region& region) {
	region.getBuilding().remove();
}

void RegionManager::update(GlobalTimer& timer) {
	current_time += timer.get_elapsed_time();
	while (!moving_armies.empty() && moving_armies.top().reach_time <= current_time) {
		MovingArmy army = moving_armies.top();
		moving_armies.pop();
		Region& end_region = get_region(std::get<0>(army.path.back()), std::get<1>(army.path.back()));
		Region& start_region = get_region(std::get<0>(army.path.front()), std::get<1>(army.path.front()));
		if (end_region.getOwner() == start_region.getOwner())
		{
			end_region.addArmy(army.amount);
		}
		else{
			int rest_army = army.amount - end_region.getArmy().getForce();
			end_region.setOwner(start_region.getOwner());
			end_region.getArmy().removeArmy(end_region.getArmy().getForce());
			end_region.getArmy().addArmy(rest_army);
			end_region.getWeapons().clear();
			clear_building(end_region);
		}
	}

	while (!moving_missles.empty() && moving_missles.top().reach_time <= current_time) {
		MovingMissle missle = moving_missles.top();
		moving_missles.pop();
		Region& end_region = get_region(std::get<0>(missle.end_point), std::get<1>(missle.end_point));

		Weapon weapon = get_weapon(missle.weapon_id);
		int damage = weapon.getDamage(missle.weapon_level);
		float damage_range = weapon.getDamageRange(missle.weapon_level);

		std::vector<Region> damaged_regions = get_damaged_regions(end_region.getPosition(), damage_range);

		while (!damaged_regions.empty())
		{
			Region region = damaged_regions.back();
			int rest_hp = region.getHp() - damage;
			if (rest_hp <= 0) {
				region.setOwner(-1);
				std::random_device rd;
				std::mt19937 gen(rd());
				std::uniform_int_distribution<> dis(200, 300);
				region.setHp(dis(gen));
				region.getWeapons().clear();
				clear_building(region);
			}
			else {
				region.setHp(rest_hp);
			}
		}
	}

	std::vector<MovingArmy> temp_armies;
	std::vector<MovingMissle> temp_missles;


	while (!moving_armies.empty()) {
		MovingArmy army = moving_armies.top();
		moving_armies.pop();
		temp_armies.push_back(army);
	}
	while (!temp_armies.empty()) {
		MovingArmy army = temp_armies.back();
		//update current position
		int side_num = army.path.size() - 1;
		int current_side = (1 - (current_time - army.reach_time) / army.time) * side_num;
		float current_x = std::get<0>(army.path[current_side]) + 1.0 * ((1 - (current_time - army.reach_time) / army.time) * side_num - current_side) + 0.5;
		float current_y = std::get<1>(army.path[current_side]) + 1.0 * ((1 - (current_time - army.reach_time) / army.time) * side_num - current_side) + 0.5;

		army.current_pos = std::make_tuple(current_x, current_y);

		moving_armies.push(army);
		temp_armies.pop_back();
	}

	while (!moving_missles.empty()) {
		MovingMissle missle = moving_missles.top();
		moving_missles.pop();
		//update current postion
		float current_x = std::get<0>(missle.end_point) - (current_time - missle.reach_time) * (std::get<0>(missle.end_point) - std::get<0>(missle.start_point)) / missle.time + 0.5;
		float current_y = std::get<1>(missle.end_point) - (current_time - missle.reach_time) * (std::get<1>(missle.end_point) - std::get<1>(missle.start_point)) / missle.time + 0.5;

		missle.current_pos = std::make_tuple(current_x, current_y);

		temp_missles.push_back(missle);
	}
	while (!temp_missles.empty()) {
		MovingMissle missle = temp_missles.back();
		moving_missles.push(missle);
		temp_missles.pop_back();
	}
}

void RegionManager::calculate_delta_resources(std::vector<int> delta_resourcce, double delta_t, int player_id) {
	int owned_regions = 0;
	Config& configer = Config::getInstance();
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			Region& region = get_region(i, j);
			int PowerStation_product = configer.getConfig({ "Building","PowerStation","Product" }).template get<std::vector<int>>()[3];
			int Refinery_product = configer.getConfig({ "Building","Refinery","Product" }).template get<std::vector<int>>()[1];
			int SteelFactory_product = configer.getConfig({ "Building","SteelFactory","Product" }).template get<std::vector<int>>()[2];
			int CivilFactory_product = configer.getConfig({ "Building","CivilFactory","Product" }).template get<std::vector<int>>()[0];
			float UpLevelFactor1 = configer.getConfig({ "Building","PowerStation","UpLevelFactor" }).template get<std::vector<float>>()[0];
			float UpLevelFactor2 = configer.getConfig({ "Building","PowerStation","UpLevelFactor" }).template get<std::vector<float>>()[1];
			if (region.getOwner() == player_id) {
				owned_regions++;
				if (region.getBuilding().getName() != "none") {
					if (region.getBuilding().getName() == "PowerStation") {
						int delta = PowerStation_product * delta_t;
						if (region.getBuilding().getLevel() == 1) {
							delta_resourcce[3] += delta;
						}
						else if (region.getBuilding().getLevel() == 2) {
							delta_resourcce[3] += delta * UpLevelFactor1;
						}
						else if (region.getBuilding().getLevel() == 3) {
							delta_resourcce[3] += delta * UpLevelFactor1 * UpLevelFactor2;
						}
					}
					else if (region.getBuilding().getName() == "Refinery") {
						int delta = Refinery_product * delta_t;
						if (region.getBuilding().getLevel() == 1) {
							delta_resourcce[1] += delta;
						}
						else if (region.getBuilding().getLevel() == 2) {
							delta_resourcce[1] += delta * UpLevelFactor1;
						}
						else if (region.getBuilding().getLevel() == 3) {
							delta_resourcce[1] += delta * UpLevelFactor1 * UpLevelFactor2;
						}
					}
					else if (region.getBuilding().getName() == "SteelFactory") {
						int delta = SteelFactory_product * delta_t;
						if (region.getBuilding().getLevel() == 1) {
							delta_resourcce[2] += delta;
						}
						else if (region.getBuilding().getLevel() == 2) {
							delta_resourcce[2] += delta * UpLevelFactor1;
						}
						else if (region.getBuilding().getLevel() == 3) {
							delta_resourcce[2] += delta * UpLevelFactor1 * UpLevelFactor2;
						}
					}
					else if (region.getBuilding().getName() == "CivilFactory") {
						int delta = CivilFactory_product * delta_t;
						if (region.getBuilding().getLevel() == 1) {
							delta_resourcce[0] += delta;
						}
						else if (region.getBuilding().getLevel() == 2) {
							delta_resourcce[0] += delta * UpLevelFactor1;
						}
						else if (region.getBuilding().getLevel() == 3) {
							delta_resourcce[0] += delta * UpLevelFactor1 * UpLevelFactor2;
						}
					}
				}
			}
		}
	}
	delta_resourcce[4] = owned_regions * 30;
}
