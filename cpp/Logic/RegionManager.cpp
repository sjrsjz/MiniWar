#include "../../header/Logic/RegionManager.h"
#include "../../header/Logic/Player.h"
#include <vector>


RegionManager::RegionManager() {
}

RegionManager::RegionManager(int width, int height) : width(width), height(height), regions(width, height), moving_missles(), moving_armies(){
	//players = std::vector<Player>(player_amount);

	//players.reserve(player_amount);

	//read configer, initialize Weapon
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			regions(x, y) = Region(x, y);
		}
	}
}

RegionManager::~RegionManager() {
}

double RegionManager::calculate_Euclidean_distance(std::tuple<int, int> start, std::tuple<int, int> end) {
	Region& start_region = get_region(std::get<0>(start), std::get<1>(start));
	Region& end_region = get_region(std::get<0>(end), std::get<1>(end));
	Point start_region_position = start_region.getPosition();
	Point end_region_position = end_region.getPosition();
	return sqrt(pow(start_region_position.getX() - end_region_position.getX(), 2) + pow(start_region_position.getY() - end_region_position.getY(), 2));
}

double RegionManager::calculate_distance(Point start, Point end, std::vector<std::tuple<int, int>>& path) {
	int start_x = std::floor(start.getX());
	int start_y = std::floor(start.getY());
	int end_x = std::floor(end.getX());
	int end_y = std::floor(end.getY());

	int id = get_region(start_x, start_y).getOwner();

	double distance = 0.f;

	Array<Region>& regions = get_regions();
	int* player_region_martix = new int[regions.get_width() * regions.get_height()];
	for (int i = 0; i < regions.get_width(); i++) {
		for (int j = 0; j < regions.get_height(); j++) {
			if (regions(i, j).getOwner() == id) {
				player_region_martix[i + j * regions.get_width()] = 1;
			}
			else
				player_region_martix[i + j * regions.get_width()] = 0;
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
		if (temp_y + 1 > 0)
		{
			cost_list[0] = NOT_AVALIABLE;
			cost_list[1] = NOT_AVALIABLE;
			cost_list[2] = NOT_AVALIABLE;
			up = false;
		}
		if (up) {
			if (player_region_martix[temp_x + (temp_y + 1) * regions.get_width()] == 1) {
				cost_list[0] = straight_cost * (1 + end_x - temp_x + end_y - temp_y);
			}
			else {
				cost_list[0] = NOT_AVALIABLE;
			}
			if (left) {
				if (player_region_martix[temp_x - 1 + (temp_y + 1) * regions.get_width()] == 1) {
					cost_list[1] = diagonal_cost + straight_cost * (end_x - temp_x + end_y - temp_y);
				}
				else {
					cost_list[1] = NOT_AVALIABLE;
				}
			}
			if (right) {
				if (player_region_martix[temp_x + 1 + (temp_y + 1) * regions.get_width()] == 1) {
					cost_list[2] = diagonal_cost + straight_cost * (end_x - temp_x + end_y - temp_y);
				}
				else {
					cost_list[2] = NOT_AVALIABLE;
				}
			}
		}
		if (down) {
			if (player_region_martix[temp_x + (temp_y - 1) * regions.get_width()] == 1) {
				cost_list[5] = straight_cost * (1 + end_x - temp_x + end_y - temp_y);
			}
			else {
				cost_list[5] = NOT_AVALIABLE;
			}
			if (left) {
				if (player_region_martix[temp_x - 1 + (temp_y - 1) * regions.get_width()] == 1) {
					cost_list[6] = diagonal_cost + straight_cost * (end_x - temp_x + end_y - temp_y);
				}
				else {
					cost_list[6] = NOT_AVALIABLE;
				}
			}
			if (right) {
				if (player_region_martix[temp_x + 1 + (temp_y - 1) * regions.get_width()] == 1) {
					cost_list[7] = diagonal_cost + straight_cost * (end_x - temp_x + end_y - temp_y);
				}
				else {
					cost_list[7] = NOT_AVALIABLE;
				}
			}
		}
		if (left) {
			if (player_region_martix[temp_x - 1 + temp_y * regions.get_width()] == 1) {
				cost_list[3] = straight_cost * (1 + end_x - temp_x + end_y - temp_y);
			}
			else {
				cost_list[3] = NOT_AVALIABLE;
			}
		}
		if (right) {
			if (player_region_martix[temp_x + 1 + temp_y * regions.get_width()] == 1) {
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
		player_region_martix[temp_x + temp_y * regions.get_width()] = 0;
	}
	return distance;
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

void RegionManager::move_army(int amount, double time, std::vector<std::tuple<int, int>>& path) {
	MovingArmy army;
	army.amount = amount;
	army.time = time;
	Region end_region = get_region(std::get<0>(path.back()), std::get<1>(path.back()));

	army.path = path;
	moving_armies.push(army);
	//count time
	//if time is up, move
	end_region.addArmy(amount);
}

void RegionManager::move_army(Point start, Point end, int amount) {
	std::vector<std::tuple<int, int>> path;

	double distance = calculate_distance(start, end, path);
	if (distance == -1.f) {
		throw "Can't find a path";
	}

	int start_x = std::floor(start.getX());
	int start_y = std::floor(start.getY());
	int end_x = std::floor(end.getX());
	int end_y = std::floor(end.getY());

	Region& start_region = get_region(start_x, start_y);
	Region& end_region = get_region(end_x, end_y);

	double time = distance / start_region.getArmy().getSpeed();

	start_region.removeArmy(amount);

	MovingArmy army;
	army.amount = amount;
	army.time = time;
	army.path = path;
	moving_armies.push(army);

	//count time
	//if time is up, move
	end_region.addArmy(amount);
}

void RegionManager::attack_region_missle(int weapon_id, Point start, Point end, double time, int damage) {
	Region& start_region = get_region(start.getX(), start.getY());
	Region& end_region = get_region(end.getX(), end.getY());
	MovingMissle missle;
	missle.damage = damage;
	missle.time = time;
	missle.start_point = std::make_tuple(start.getX(), start.getY());
	missle.end_point = std::make_tuple(end.getX(), end.getY());
	moving_missles.push(missle);
	//count time
	//if time is up, attack
	//if time is up, remove missle

	int rest_hp = end_region.getHp() - damage;
	if (rest_hp <= 0) {
		end_region.setOwner(-1);
		end_region.setHp(0);
		end_region.getWeapons().clear();
		clear_building(end_region);
	}
	else {
		end_region.setHp(rest_hp);
	}
}

void RegionManager::attack_region_army(Point start, Point end, int amount) {
	Region& start_region = get_region(start.getX(), start.getY());
	Region& end_region = get_region(end.getX(), end.getY());
	MovingArmy army;
	army.amount = amount;
	moving_armies.push(army);
	
	//count time
	//if time is up, attack
	//if time is up, remove army
	
	int rest_army = amount - end_region.getArmy().getForce();
	end_region.setOwner(start_region.getOwner());
	end_region.getArmy().removeArmy(end_region.getArmy().getForce());
	end_region.getArmy().addArmy(rest_army);
	end_region.getWeapons().clear();
	clear_building(end_region);
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
