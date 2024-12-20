#include "../../header/Logic/RegionManager.h"
#include "../../header/Logic/Player.h"
#include <vector>

struct MovingArmy {
	int amount;
	double time;
	std::vector<std::tuple<int,int>> path;
};

struct MovingMissle {
	int damage;
	double time;
	std::tuple<int, int> start_point;
	std::tuple<int, int> end_point;
};

RegionManager::RegionManager(int width, int height, int player_amount) : regions(width, height), moving_missles(), moving_armies(){
	players = std::vector<Player>(player_amount);

	players.reserve(player_amount);
	for (int i = 0; i < player_amount; i++) {
		players.emplace_back(*this, i);//create player in vector
	}

	//read configer, initialize Weapon

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			regions(x, y) = Region(x, y);
		}
	}
}

RegionManager::~RegionManager() {
}

Weapon& RegionManager::get_weapon(int id) {
	return weapons[id];
}

void RegionManager::move_army(int amount, double time, std::vector<std::tuple<int, int>>& path) {
	MovingArmy army;
	army.amount = amount;
	army.time = time;
	Region end_region = get_region(std::get<0>(path.back()), std::get<1>(path.back()));

	army.path = path;
	moving_armies.push_back(army);
	//count time
	//if time is up, move
	end_region.addArmy(amount);
}

void RegionManager::attack_region(int weapon_id, Point start, Point end, double time, int damage) {
	MovingMissle missle;
	missle.damage = damage;
	missle.time = time;
	missle.start_point = std::make_tuple(start.getX(), start.getY());
	missle.end_point = std::make_tuple(end.getX(), end.getY());
	moving_missles.push_back(missle);
	//count time
	//if time is up, attack
	//if time is up, remove missle
}

void RegionManager::owner_alter() {
	//need original region
	//need target region
	//change owner
}

Array<Region> RegionManager::get_regions() {
	return regions;
}

Region& RegionManager::get_region(int x, int y) {
	return regions(x, y);
}