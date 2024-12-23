#include "../../header/Logic/RegionManager.h"
#include "../../header/Logic/Player.h"
#include <vector>



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
	moving_armies.push_back(army);
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
	moving_missles.push_back(missle);
	//count time
	//if time is up, attack
	//if time is up, remove missle
}

void RegionManager::attack_region_army(Point start, Point end, int amount) {
	Region& start_region = get_region(start.getX(), start.getY());
	Region& end_region = get_region(end.getX(), end.getY());
	MovingArmy army;
	army.amount = amount;
	Region end_region = get_region(end.getX(), end.getY());
	moving_armies.push_back(army);
	
	//count time
	//if time is up, attack
	//if time is up, remove army
	
	int rest_army = amount - end_region.getArmy().getForce();
	end_region.setOwner(start_region.getOwner());
	end_region.getArmy().removeArmy(end_region.getArmy().getForce());
	end_region.getArmy().addArmy(rest_army);
	clear_building(end_region);
}

Array<Region>& RegionManager::get_regions() {
	return regions;
}

Region& RegionManager::get_region(int x, int y) {
	return regions(x, y);
}

void RegionManager::clear_building(Region& region) {
	region.getBuilding().remove();
}