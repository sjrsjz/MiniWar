#include "../../header/Logic/Player.h"
#include "../../header/Logic/RegionManager.h"

Player:: Player(RegionManager& Manager, int id) : regionmanager(Manager), id(id){
    gold = 0;
    oil = 0;
    electricity = 0;
    labor = 0;
    steel = 0;
    arm_level = {1, 0, 0, 0};
    institution_level_limit = {1, 1, 1, 1, 1, 0};
}

Player:: ~Player(){
}

double Player::calculate_Euclidean_distance(std::tuple<int, int> start, std::tuple<int, int> end) {
    Region start_region = regionmanager.get_region(std::get<0>(start), std::get<1>(start));
    Region end_region = regionmanager.get_region(std::get<0>(end), std::get<1>(end));
    Point start_region_position = start_region.getPosition();
    Point end_region_position = end_region.getPosition();
    return sqrt(pow(start_region_position.getX() - end_region_position.getX(), 2) + pow(start_region_position.getY() - end_region_position.getY(), 2));
}

double Player::calculate_distance(Point start, Point end, std::vector<std::tuple<int, int>>& path) {
	double distance = 0;

    Array<Region> regions = regionmanager.get_regions();
    int *player_region_martix = new int[regions.get_width() * regions.get_height()];
    for(int i = 0; i<regions.get_width(); i++){
        for(int j = 0; j<regions.get_height(); j++){
            if (regions(i,j).getOwner() == id) {
                player_region_martix[i + j * regions.get_width()] = 1;
            }
            else
            player_region_martix[i + j * regions.get_width()] = 0;
        }
    }

    int start_x = std::floor(start.getX());
    int start_y = std::floor(start.getY());
    int end_x = std::floor(end.getX());
    int end_y = std::floor(end.getY());

    int temp_x = start_x;
    int temp_y = start_y;

    static int NOT_AVALIABLE = 1000000;
    static int straight_cost = 10;
    static int diagonal_cost = 14;

    int cost_list[8] = {};//0: left up 1: up 2: right up 3: left 4: right 5: left down 6: down 7: right down

    while(temp_x!= end_x && temp_y != end_y)
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
				cost_list[0] = straight_cost*(1 + end_x - temp_x + end_y - temp_y);
			}
            else {
				cost_list[0] = NOT_AVALIABLE;
			}
            if (left) {
				if (player_region_martix[temp_x - 1 + (temp_y + 1) * regions.get_width()] == 1) {
					cost_list[1] = diagonal_cost + straight_cost*(end_x - temp_x + end_y - temp_y);
				}
                else {
					cost_list[1] = NOT_AVALIABLE;
				}
            }
            if (right) {
				if (player_region_martix[temp_x + 1 + (temp_y + 1) * regions.get_width()] == 1) {
					cost_list[2] = diagonal_cost + straight_cost*(end_x - temp_x + end_y - temp_y);
				}
                else {
					cost_list[2] = NOT_AVALIABLE;
                }
            }
        }
        if (down) {
			if (player_region_martix[temp_x + (temp_y - 1) * regions.get_width()] == 1) {
				cost_list[5] = straight_cost*(1 + end_x - temp_x + end_y - temp_y);
			}
            else {
				cost_list[5] = NOT_AVALIABLE;
			}
            if (left) {
				if (player_region_martix[temp_x - 1 + (temp_y - 1) * regions.get_width()] == 1) {
					cost_list[6] = diagonal_cost + straight_cost*(end_x - temp_x + end_y - temp_y);
				}
                else {
					cost_list[6] = NOT_AVALIABLE;
				}
            }
            if (right) {
				if (player_region_martix[temp_x + 1 + (temp_y - 1) * regions.get_width()] == 1) {
					cost_list[7] = diagonal_cost + straight_cost*(end_x - temp_x + end_y - temp_y);
				}
                else {
					cost_list[7] = NOT_AVALIABLE;
				}
            }
        }
        if (left) {
			if (player_region_martix[temp_x - 1 + temp_y * regions.get_width()] == 1) {
				cost_list[3] = straight_cost*(1 + end_x - temp_x + end_y - temp_y);
			}
            else {
				cost_list[3] = NOT_AVALIABLE;
			}
        }
        if (right) {
			if (player_region_martix[temp_x + 1 + temp_y * regions.get_width()] == 1) {
				cost_list[4] = straight_cost*(1 + end_x - temp_x + end_y - temp_y);
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
			return -1;
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

int Player:: get_gold(){
    return gold;
}

int Player:: get_electricity(){
    return electricity;
}

int Player:: get_labor(){
    return labor;
}

int Player:: get_steel(){
    return steel;
}

int Player:: get_oil(){
    return oil;
}

int Player::get_capital_x() {
	return std::floor(std::get<0>(capital));
}

int Player::get_capital_y() {
	return std::floor(std::get<1>(capital));
}

void Player:: gold_cost(int cost){
    if (gold < cost){
        throw "Not enough gold";
    }
    gold -= cost;
}

void Player:: oil_cost(int cost){
    if (oil < cost){
        throw "Not enough oil";
    }
    oil -= cost;
}

void Player:: electricity_cost(int cost){
    if (electricity < cost){
        throw "Not enough electricity";
    }
    electricity -= cost;
}

void Player:: labor_cost(int cost){
    if (labor < cost){
        throw "Not enough labor";
    }
    labor -= cost;
}

void Player:: steel_cost(int cost){
    if (steel < cost){
        throw "Not enough steel";
    }
    steel -= cost;
}

void Player:: add_gold(int amount){
    gold += amount;
}

void Player:: add_oil(int amount){
    oil += amount;
}

void Player:: add_electricity(int amount){
    electricity += amount;
}

void Player:: add_labor(int amount){
    labor += amount;
}

void Player:: add_steel(int amount){
    steel += amount;
}

int get_building_level_limit(std::string name) {
    if (name == "PowerStation") return 0;
    if (name == "SteelFactory") return 1;
	if (name == "Refinery") return 2;
	if (name == "CivilFactory") return 3;
	if (name == "MilitaryFactory") return 4;
	return -1;
}

void Player:: move_army(Point start, Point end, int amount){
    std::vector<std::tuple<int, int>> path;
    double distance = calculate_distance(start, end, path);
    if (distance == -1) {
        throw "Can't find a path";
    }

    int start_x = std::floor(start.getX());
    int start_y = std::floor(start.getY());
    int end_x = std::floor(end.getX());
    int end_y = std::floor(end.getY());

    Region start_region = regionmanager.get_region(start_x, start_y);
    Region end_region = regionmanager.get_region(end_x, end_y);

    double time = distance / start_region.getArmy().getSpeed();

    start_region.removeArmy(amount);
    //create a new army to move, when time is up, this army shall be destroyed, and the end region add this army's force
	regionmanager.move_army(amount, time, path);
}

void Player::attack(Point start, Point end, int weapon_id) {
    double distance = sqrt(pow(start.getX() - end.getX(), 2) + pow(start.getY() - end.getY(), 2));

    int start_x = std::floor(start.getX());
    int start_y = std::floor(start.getY());

    Region start_region = regionmanager.get_region(start_x, start_y);

	Weapon weapon = regionmanager.get_weapon(weapon_id);

    double time = distance / weapon.getAttackSpeed(arm_level[id+1]);
	int damage = weapon.getDamage(arm_level[id + 1]);

	start_region.removeWeapon(weapon_id);

	regionmanager.attack_region(weapon_id, start, end, time, damage);
}

void Player::build(std::string building_name, Point location) {
	Region region = regionmanager.get_region(std::floor(location.getX()), std::floor(location.getY()));
	if (region.getOwner() != id) {
		throw "Not your region";
	}
    if (region.getBuilding().getName() == "none") {
		throw "Already has a building";
	}
    //wait for configure.h complete
}

void Player::upgrade_building(Point location) {
	Region region = regionmanager.get_region(std::floor(location.getX()), std::floor(location.getY()));
    if (region.getBuilding().getName() == "none") {
        throw "No building exits";
    }
	if (region.getOwner() != id) {
		throw "Not your region";
	}

	//wait for configure.h complete
	//make sure cost is enough, then uplevel
    if (!region.getBuilding().upLevel(institution_level_limit[get_building_level_limit(region.getBuilding().getName())])) {
        throw "Already reach your level limit!";
    }
}

void Player::remove_building(Point location) {
	Region region = regionmanager.get_region(std::floor(location.getX()), std::floor(location.getY()));
    if (region.getOwner() != id) {
		throw "Not your region";
	}
    if (region.getBuilding().getName() == "none") {
		throw "No building exits";
	}
	region.removeBuilding();
	//return the cost of the building
}

void Player::research(int selection) {
	Region region = regionmanager.get_region(get_capital_x(), get_capital_y());
    if (region.getOwner() != id) {
		throw "Not your capital";
	}
    if (region.getBuilding().getName() != "ResearchInstitution") {
		throw "No research center";
	}
	//wait for configure.h complete
	//make sure cost is enough, then research
}

void Player:: update(Timer timer){

}
