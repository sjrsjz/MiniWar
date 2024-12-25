#include "../../header/Logic/Player.h"
#include "../../header/Logic/RegionManager.h"

Player:: Player(): regionmanager(RegionManager::getInstance()){
    id = 0;
}

Player:: ~Player() {
}

//double Player::calculate_Euclidean_distance(std::tuple<int, int> start, std::tuple<int, int> end) {
//    Region& start_region = regionmanager.get_region(std::get<0>(start), std::get<1>(start));
//    Region& end_region = regionmanager.get_region(std::get<0>(end), std::get<1>(end));
//    Point start_region_position = start_region.getPosition();
//    Point end_region_position = end_region.getPosition();
//    return sqrt(pow(start_region_position.getX() - end_region_position.getX(), 2) + pow(start_region_position.getY() - end_region_position.getY(), 2));
//}
//
//double Player::calculate_distance(Point start, Point end, std::vector<std::tuple<int, int>>& path) {
//	double distance = 0.f;
//
//    Array<Region>& regions = regionmanager.get_regions();
//    int *player_region_martix = new int[regions.get_width() * regions.get_height()];
//    for(int i = 0; i<regions.get_width(); i++){
//        for(int j = 0; j<regions.get_height(); j++){
//            if (regions(i,j).getOwner() == id) {
//                player_region_martix[i + j * regions.get_width()] = 1;
//            }
//            else
//            player_region_martix[i + j * regions.get_width()] = 0;
//        }
//    }
//
//    int start_x = std::floor(start.getX());
//    int start_y = std::floor(start.getY());
//    int end_x = std::floor(end.getX());
//    int end_y = std::floor(end.getY());
//
//    int temp_x = start_x;
//    int temp_y = start_y;
//
//    static int NOT_AVALIABLE = 1000000;
//    static int straight_cost = 10;
//    static int diagonal_cost = 14;
//
//    int cost_list[8] = {};//0: left up 1: up 2: right up 3: left 4: right 5: left down 6: down 7: right down
//
//    while(temp_x!= end_x && temp_y != end_y)
//    {
//        path.push_back(std::make_tuple(temp_x, temp_y));
//
//        bool up = true;
//		bool down = true;
//		bool left = true;
//		bool right = true;
//        if (temp_x - 1 < 0)
//        {
//			cost_list[0] = NOT_AVALIABLE;
//			cost_list[3] = NOT_AVALIABLE;
//			cost_list[5] = NOT_AVALIABLE;
//			left = false;
//		}
//        if (temp_x + 1 > regions.get_width())
//        {
//			cost_list[2] = NOT_AVALIABLE;
//			cost_list[4] = NOT_AVALIABLE;
//			cost_list[7] = NOT_AVALIABLE;
//			right = false;
//        }
//		if (temp_y - 1 < 0)
//		{
//			cost_list[5] = NOT_AVALIABLE;
//			cost_list[6] = NOT_AVALIABLE;
//			cost_list[7] = NOT_AVALIABLE;
//			down = false;
//		}
//        if (temp_y + 1 > 0)
//        {
//			cost_list[0] = NOT_AVALIABLE;
//			cost_list[1] = NOT_AVALIABLE;
//			cost_list[2] = NOT_AVALIABLE;
//			up = false;
//        }
//        if (up) {
//			if (player_region_martix[temp_x + (temp_y + 1) * regions.get_width()] == 1) {
//				cost_list[0] = straight_cost*(1 + end_x - temp_x + end_y - temp_y);
//			}
//            else {
//				cost_list[0] = NOT_AVALIABLE;
//			}
//            if (left) {
//				if (player_region_martix[temp_x - 1 + (temp_y + 1) * regions.get_width()] == 1) {
//					cost_list[1] = diagonal_cost + straight_cost*(end_x - temp_x + end_y - temp_y);
//				}
//                else {
//					cost_list[1] = NOT_AVALIABLE;
//				}
//            }
//            if (right) {
//				if (player_region_martix[temp_x + 1 + (temp_y + 1) * regions.get_width()] == 1) {
//					cost_list[2] = diagonal_cost + straight_cost*(end_x - temp_x + end_y - temp_y);
//				}
//                else {
//					cost_list[2] = NOT_AVALIABLE;
//                }
//            }
//        }
//        if (down) {
//			if (player_region_martix[temp_x + (temp_y - 1) * regions.get_width()] == 1) {
//				cost_list[5] = straight_cost*(1 + end_x - temp_x + end_y - temp_y);
//			}
//            else {
//				cost_list[5] = NOT_AVALIABLE;
//			}
//            if (left) {
//				if (player_region_martix[temp_x - 1 + (temp_y - 1) * regions.get_width()] == 1) {
//					cost_list[6] = diagonal_cost + straight_cost*(end_x - temp_x + end_y - temp_y);
//				}
//                else {
//					cost_list[6] = NOT_AVALIABLE;
//				}
//            }
//            if (right) {
//				if (player_region_martix[temp_x + 1 + (temp_y - 1) * regions.get_width()] == 1) {
//					cost_list[7] = diagonal_cost + straight_cost*(end_x - temp_x + end_y - temp_y);
//				}
//                else {
//					cost_list[7] = NOT_AVALIABLE;
//				}
//            }
//        }
//        if (left) {
//			if (player_region_martix[temp_x - 1 + temp_y * regions.get_width()] == 1) {
//				cost_list[3] = straight_cost*(1 + end_x - temp_x + end_y - temp_y);
//			}
//            else {
//				cost_list[3] = NOT_AVALIABLE;
//			}
//        }
//        if (right) {
//			if (player_region_martix[temp_x + 1 + temp_y * regions.get_width()] == 1) {
//				cost_list[4] = straight_cost*(1 + end_x - temp_x + end_y - temp_y);
//			}
//            else {
//				cost_list[4] = NOT_AVALIABLE;
//			}
//        }
//
//		int min_cost = NOT_AVALIABLE, min_way = 0;
//        for (int i = 0; i < 8; i++) {
//			if (cost_list[i] < min_cost) {
//				min_cost = cost_list[i];
//				min_way = i;
//			}
//        }
//		if (min_cost == NOT_AVALIABLE) {
//			return -1.f;
//		}
//
//        switch (min_way) {
//        case 0:
//            temp_y++;
//            distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x, temp_y - 1));
//            break;
//        case 1:
//            temp_x--;
//            temp_y++;
//            distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x + 1, temp_y - 1));
//            break;
//        case 2:
//            temp_x++;
//            temp_y++;
//            distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x - 1, temp_y - 1));
//            break;
//        case 3:
//            temp_x--;
//            distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x + 1, temp_y));
//            break;
//        case 4:
//            temp_x++;
//            distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x - 1, temp_y));
//            break;
//        case 5:
//            temp_y--;
//            distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x, temp_y + 1));
//            break;
//        case 6:
//            temp_x--;
//            temp_y--;
//            distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x + 1, temp_y + 1));
//            break;
//        case 7:
//            temp_x++;
//            temp_y--;
//            distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x - 1, temp_y + 1));
//            break;
//        }
//		player_region_martix[temp_x + temp_y * regions.get_width()] = 0;
//    }
//    return distance;
//}

int Player:: get_gold(){
    return gold;
}

int Player:: get_electricity(){
    return electricity;
}

int Player:: get_labor_limit(){
    return labor_limit;
}

int Player::get_ocupied_labor() {
	return ocupied_labor;
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
        throw new std::exception("Not enough gold");
    }
    gold -= cost;
}

void Player:: oil_cost(int cost){
    if (oil < cost){
        throw new std::exception("Not enough oil");
    }
    oil -= cost;
}

void Player:: electricity_cost(int cost){
    if (electricity < cost){
        throw new std::exception("Not enough electricity");
    }
    electricity -= cost;
}

void Player:: labor_cost(int cost){
    if (labor_limit - ocupied_labor < cost){
        throw new std::exception("Not enough labor");
    }
    ocupied_labor += cost;
}

void Player:: steel_cost(int cost){
    if (steel < cost){
        throw new std::exception("Not enough steel");
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
    labor_limit += amount; 
}// not sure

void Player:: add_steel(int amount){
    steel += amount;
}

int Player::get_building_level_limit(std::string name) {
    if (name == "PowerStation") return 0;
	if (name == "Refinery") return 1;
    if (name == "SteelFactory") return 2;
	if (name == "CivilFactory") return 3;
	if (name == "MilitaryFactory") return 4;
	return -1;
}

void Player:: move_army(Operation operation, int amount){
 //   std::vector<std::tuple<int, int>> path;
 //   double distance = calculate_distance(start, end, path);
 //   if (distance == -1) {
 //       throw "Can't find a path";
 //   }

 //   int start_x = std::floor(start.getX());
 //   int start_y = std::floor(start.getY());
 //   int end_x = std::floor(end.getX());
 //   int end_y = std::floor(end.getY());

 //   Region& start_region = regionmanager.get_region(start_x, start_y);
 //   Region& end_region = regionmanager.get_region(end_x, end_y);

 //   double time = distance / start_region.getArmy().getSpeed();

 //   start_region.removeArmy(amount);
 //   //create a new army to move, when time is up, this army shall be destroyed, and the end region add this army's force
	//regionmanager.move_army(amount, time, path);
	Point start = operation.getStart();
	Point end = operation.getEnd();

    int start_x = std::floor(start.getX());
    int start_y = std::floor(start.getY());
    int end_x = std::floor(end.getX());
    int end_y = std::floor(end.getY());

	Region& start_region = regionmanager.get_region(start_x, start_y);
	Region& end_region = regionmanager.get_region(end_x, end_y);

	if (start_region.getOwner() != id || end_region.getOwner()) {
		throw "Not your region";
	}
	if (start_region.getArmy().getForce() < amount) {
		throw "Not enough army";
	}

	regionmanager.move_army(start, end, amount, arm_level[0]);
}

void Player::attack(Operation operation) {
	Point start = operation.getStart();
	Point end = operation.getEnd();
    double distance = sqrt(pow(start.getX() - end.getX(), 2) + pow(start.getY() - end.getY(), 2));

    int start_x = std::floor(start.getX());
    int start_y = std::floor(start.getY());

    Region& start_region = regionmanager.get_region(start_x, start_y);

	int weapon_id = 0;

	switch (operation.getOp())
	{
	case Operator::Weapon0Attack:
		weapon_id = 0;
		break;
	case Operator::Weapon1Attack:
		weapon_id = 1;
		break;
	case Operator::Weapon2Attack:
		weapon_id = 2;
		break;
	}

	Weapon weapon = regionmanager.get_weapon(weapon_id);

	std::tuple<float, float> AttackRange= weapon.getAttackRange();

	if (distance > std::get<1>(AttackRange) || distance < std::get<0>(AttackRange)) {
		throw "Not in attack range";
	}

    double time = distance / weapon.getAttackSpeed(arm_level[id+1]);

	start_region.removeWeapon(weapon_id);

	regionmanager.attack_region_missle(weapon_id, arm_level[weapon_id + 1], start, end, time);
} //should detect if the weapon can reach the target

void Player::build(Operation operation) {

	Point location = operation.getCur();
	std::string building_name = "";

	switch (operation.getOp())
	{
	case Operator::SetPowerStation:
		building_name = "PowerStation";
		break;
	case Operator::SetRefinery:
		building_name = "Refinery";
		break;
	case Operator::SetSteelFactory:
		building_name = "SteelFactory";
		break;
	case Operator::SetCivilFactory:
		building_name = "CivilFactory";
		break;
	case Operator::SetMilitaryFactory:
		building_name = "MilitaryFactory";
		break;
	}

	Region& region = regionmanager.get_region(std::floor(location.getX()), std::floor(location.getY()));
	if (region.getOwner() != id) {
		throw "Not your region";
	}
    if (region.getBuilding().getName() == "none") {
		throw "Already has a building";
	}
    //wait for configure.h complete
	Config& configer = Config::getInstance();
	json BuildCost = configer.getConfig({ "Building",building_name, "BuildCost" });
	std::vector<int> Level1Cost = BuildCost.template get<std::vector<int>>();
	if (gold < Level1Cost[0] || oil < Level1Cost[1] || steel < Level1Cost[2] || electricity < Level1Cost[3] || labor_limit - ocupied_labor < Level1Cost[4]) {
		throw "Not enough resource";
	}
	gold -= Level1Cost[0];
	oil -= Level1Cost[1];
	steel -= Level1Cost[2];
	electricity -= Level1Cost[3];
	ocupied_labor += Level1Cost[4];
	Building building(building_name);
	region.setBuilding(building);
}

void Player::upgrade_building(Operation operation) {
	Point location = operation.getCur();
	Region& region = regionmanager.get_region(std::floor(location.getX()), std::floor(location.getY()));
    if (region.getBuilding().getName() == "none") {
        throw "No building exits";
    }
	if (region.getOwner() != id) {
		throw "Not your region";
	}

	//wait for configure.h complete
	//make sure cost is enough, then uplevel
    if (region.getBuilding().getLevel() == institution_level_limit[get_building_level_limit(region.getBuilding().getName())]) {
        throw "Already reach your level limit!";
    }

	Config& configer = Config::getInstance();
    switch (region.getBuilding().getLevel())
    {
    case 1:
		gold -= configer.getConfig({ "Building",region.getBuilding().getName(), "UpLevelCost1", "Gold" }).get<int>();
		oil -= configer.getConfig({ "Building",region.getBuilding().getName(), "UpLevelCost1", "Oil" }).get<int>();
		steel -= configer.getConfig({ "Building",region.getBuilding().getName(), "UpLevelCost1", "Steel" }).get<int>();
		electricity -= configer.getConfig({ "Building",region.getBuilding().getName(), "UpLevelCost1", "Electricity" }).get<int>();
		ocupied_labor += configer.getConfig({ "Building",region.getBuilding().getName(), "UpLevelCost1", "Labor" }).get<int>();
		break;  
	case 2:
		gold -= configer.getConfig({ "Building",region.getBuilding().getName(), "UpLevelCost2", "Gold" }).get<int>();
		oil -= configer.getConfig({ "Building",region.getBuilding().getName(), "UpLevelCost2", "Oil" }).get<int>();
		steel -= configer.getConfig({ "Building",region.getBuilding().getName(), "UpLevelCost2", "Steel" }).get<int>();
		electricity -= configer.getConfig({ "Building",region.getBuilding().getName(), "UpLevelCost2", "Electricity" }).get<int>();
		ocupied_labor += configer.getConfig({ "Building",region.getBuilding().getName(), "UpLevelCost2", "Labor" }).get<int>();
		break;
    }

    region.getBuilding().upLevel(institution_level_limit[get_building_level_limit(region.getBuilding().getName())]);
}

void Player::remove_building(Operation operation) {
	Point location = operation.getCur();
	Region& region = regionmanager.get_region(std::floor(location.getX()), std::floor(location.getY()));
    if (region.getOwner() != id) {
		throw "Not your region";
	}
    if (region.getBuilding().getName() == "none") {
		throw "No building exits";
	}
	region.removeBuilding();
	//return the cost of the building

	Building& building = region.getBuilding();
	Config& configer = Config::getInstance();
    json BuildCost = configer.getConfig({ "Building",building.getName(), "BuildCost" });
    json UpLevelCost1 = configer.getConfig({ "Building",building.getName(), "UpLevelCost1" });
    json UpLevelCost2 = configer.getConfig({ "Building",building.getName(), "UpLevelCost2" });
    std::vector<int> Level1Cost = BuildCost.template get<std::vector<int>>();
    std::vector<int> Level2Cost = UpLevelCost1.template get<std::vector<int>>();
    std::vector<int> Level3Cost = UpLevelCost2.template get<std::vector<int>>();
    switch (building.getLevel())
    {
    case 1:
		gold += std::floor(0.6 * Level1Cost[0]);
		oil += std::floor(0.6 * Level1Cost[1]);
		steel += std::floor(0.6 * Level1Cost[2]);
		electricity += std::floor(0.6 * Level1Cost[3]);
		ocupied_labor -= Level1Cost[4];
		break;
    case 2:
        gold += std::floor(0.6 * (Level1Cost[0] + Level2Cost[0]));
        oil += std::floor(0.6 * (Level1Cost[1] + Level2Cost[1]));
        steel += std::floor(0.6 * (Level1Cost[2] + Level2Cost[2]));
        electricity += std::floor(0.6 * (Level1Cost[3] + Level2Cost[3]));
        ocupied_labor -= Level1Cost[4] + Level2Cost[4];
        break;
    case 3:
		gold += std::floor(0.6 * (Level1Cost[0] + Level2Cost[0] + Level3Cost[0]));
		oil += std::floor(0.6 * (Level1Cost[1] + Level2Cost[1] + Level3Cost[1]));
		steel += std::floor(0.6 * (Level1Cost[2] + Level2Cost[2] + Level3Cost[2]));
		electricity += std::floor(0.6 * (Level1Cost[3] + Level2Cost[3] + Level3Cost[3]));
		ocupied_labor -= Level1Cost[4] + Level2Cost[4] + Level3Cost[4];
		break;
    }
}

void Player::set_research(Operation operation) {
	Config& configer = Config::getInstance();
	int cost = configer.getConfig({ "ResearchInstitution","BuildCost" }).get<int>();
	if (gold < cost) {
		throw "Not enough gold";
	}
	gold -= cost;
	have_research_institution = true;
}

void Player::research(Operation operation) {
	if (!have_research_institution) {
		throw "Research institution not built";
	}
	//wait for configure.h complete
	//make sure cost is enough, then research
    Config& configer = Config::getInstance();
	std::vector<int> Uplevelcost_PowerStation = configer.getConfig({ "ResearchInstitution","OUpLevelCost","PowerStation" }).template get<std::vector<int>>();
	std::vector<int> Uplevelcost_Refinery = configer.getConfig({ "ResearchInstitution","OUpLevelCost","Refinery" }).template get<std::vector<int>>();
	std::vector<int> Uplevelcost_SteelFactory = configer.getConfig({ "ResearchInstitution","OUpLevelCost","SteelFactory" }).template get<std::vector<int>>();
	std::vector<int> Uplevelcost_CivilFactory = configer.getConfig({ "ResearchInstitution","OUpLevelCost","CivilFactory" }).template get<std::vector<int>>();
	std::vector<int> Uplevelcost_MilitaryFactory = configer.getConfig({ "ResearchInstitution","OUpLevelCost","MilitaryFactory" }).template get<std::vector<int>>();

	std::vector<int> Uplevelcost_Army = configer.getConfig({ "ResearchInstitution","OUpLevelCost","Army" }).template get<std::vector<int>>();
	std::vector<int> Uplevelcost_CM = configer.getConfig({ "ResearchInstitution","OUpLevelCost","0" }).template get<std::vector<int>>();
	std::vector<int> Uplevelcost_MRBM = configer.getConfig({ "ResearchInstitution","OUpLevelCost","1" }).template get<std::vector<int>>();
	std::vector<int> Uplevelcost_ICBM = configer.getConfig({ "ResearchInstitution","OUpLevelCost","2" }).template get<std::vector<int>>();

	switch (operation.getOp()) {
	case Operator::PowerStationUpLevel:
		if (institution_level_limit[0] == 3) {
			throw "Already reach max level";
		}
		else {
			if (institution_level_limit[0] == 1) {
				if (gold < Uplevelcost_PowerStation[0]) {
					throw "Not enough gold";
				}
				else {
					gold -= Uplevelcost_PowerStation[0];
					institution_level_limit[0] = 2;
				}
			}
			if (institution_level_limit[0] == 2) {
				if (gold < Uplevelcost_PowerStation[1]) {
					throw "Not enough gold";
				}
				else {
					gold -= Uplevelcost_PowerStation[1];
					institution_level_limit[0] = 3;
				}
			}
		}
		break;
	case Operator::RefineryUpLevel:
		if (institution_level_limit[1] == 3) {
			throw "Already reach max level";
		}
		else {
			if (institution_level_limit[1] == 1) {
				if (gold < Uplevelcost_Refinery[0]) {
					throw "Not enough gold";
				}
				else {
					gold -= Uplevelcost_Refinery[0];
					institution_level_limit[1] = 2;
				}
			}
			if (institution_level_limit[1] == 2) {
				if (gold < Uplevelcost_Refinery[1]) {
					throw "Not enough gold";
				}
				else {
					gold -= Uplevelcost_Refinery[1];
					institution_level_limit[1] = 3;
				}
			}
		}
		break;
	case Operator::SteelFactoryUpLevel:
		if (institution_level_limit[2] == 3) {
			throw "Already reach max level";
		}
		else {
			if (institution_level_limit[2] == 1) {
				if (gold < Uplevelcost_SteelFactory[0]) {
					throw "Not enough gold";
				}
				else {
					gold -= Uplevelcost_SteelFactory[0];
					institution_level_limit[2] = 2;
				}
			}
			if (institution_level_limit[2] == 2) {
				if (gold < Uplevelcost_SteelFactory[1]) {
					throw "Not enough gold";
				}
				else {
					gold -= Uplevelcost_SteelFactory[1];
					institution_level_limit[2] = 3;
				}
			}
		}
		break;
	case Operator::CivilFactoryUpLevel:
		if (institution_level_limit[3] == 3) {
			throw "Already reach max level";
		}
		else {
			if (institution_level_limit[3] == 1) {
				if (gold < Uplevelcost_CivilFactory[0]) {
					throw "Not enough gold";
				}
				else {
					gold -= Uplevelcost_CivilFactory[0];
					institution_level_limit[3] = 2;
				}
			}
			if (institution_level_limit[3] == 2) {
				if (gold < Uplevelcost_CivilFactory[1]) {
					throw "Not enough gold";
				}
				else {
					gold -= Uplevelcost_CivilFactory[1];
					institution_level_limit[3] = 3;
				}
			}
		}
		break;
	case Operator::MilitaryFactoryUpLevel:
		if (institution_level_limit[4] == 3) {
			throw "Already reach max level";
		}
		else {
			if (institution_level_limit[4] == 1) {
				if (gold < Uplevelcost_MilitaryFactory[0]) {
					throw "Not enough gold";
				}
				else {
					gold -= Uplevelcost_MilitaryFactory[0];
					institution_level_limit[4] = 2;
				}
			}
			if (institution_level_limit[4] == 2) {
				if (gold < Uplevelcost_MilitaryFactory[1]) {
					throw "Not enough gold";
				}
				else {
					gold -= Uplevelcost_MilitaryFactory[1];
					institution_level_limit[4] = 3;
				}
			}
		}
		break;
	case Operator::ArmyUpLevel:
		if (arm_level[0] == 3) {
			throw "Already reach max level";
		}
		else {
			if (arm_level[0] == 1) {
				if (gold < Uplevelcost_Army[0]) {
					throw "Not enough gold";
				}
				else {
					gold -= Uplevelcost_Army[0];
					arm_level[0] = 2;
				}
			}
			if (arm_level[0] == 2) {
				if (gold < Uplevelcost_Army[1]) {
					throw "Not enough gold";
				}
				else {
					gold -= Uplevelcost_Army[1];
					arm_level[0] = 3;
				}
			}
		}
		break;
	case Operator::Weapon0UpLevel:
		if (arm_level[1] == 3) {
			throw "Already reach max level";
		}
		else {
			if (arm_level[1] == 1) {
				if (gold < Uplevelcost_CM[0]) {
					throw "Not enough gold";
				}
				else {
					gold -= Uplevelcost_CM[0];
					arm_level[1] = 2;
				}
			}
			if (arm_level[1] == 2) {
				if (gold < Uplevelcost_CM[1]) {
					throw "Not enough gold";
				}
				else {
					gold -= Uplevelcost_CM[1];
					arm_level[1] = 3;
				}
			}	
		}
		break;
	case Operator::Weapon1UpLevel:
		if (arm_level[2] == 3) {
			throw "Already reach max level";
		}
		else {
			if (arm_level[2] == 1) {
				if (gold < Uplevelcost_MRBM[0]) {
					throw "Not enough gold";
				}
				else {
					gold -= Uplevelcost_MRBM[0];
					arm_level[2] = 2;
				}
			}
			if (arm_level[2] == 2) {
				if (gold < Uplevelcost_MRBM[1]) {
					throw "Not enough gold";
				}
				else {
					gold -= Uplevelcost_MRBM[1];
					arm_level[2] = 3;
				}
			}
		}
		break;
	case Operator::Weapon2UpLevel:
		if (arm_level[3] == 3) {
			throw "Already reach max level";
		}
		else {
			if (arm_level[3] == 1) {
				if (gold < Uplevelcost_ICBM[0]) {
					throw "Not enough gold";
				}
				else {
					gold -= Uplevelcost_ICBM[0];
					arm_level[3] = 2;
				}
			}
			if (arm_level[3] == 2) {
				if (gold < Uplevelcost_ICBM[1]) {
					throw "Not enough gold";
				}
				else {
					gold -= Uplevelcost_ICBM[1];
					arm_level[3] = 3;
				}
			}
		}
		break;
	}
}

void Player::product(Operation operation) {
	Point start = operation.getStart();
	Point end = operation.getEnd();
	Region& start_region = regionmanager.get_region(std::floor(start.getX()), std::floor(start.getY()));
	Region& end_region = regionmanager.get_region(std::floor(end.getX()), std::floor(end.getY()));

	if (start_region.getOwner() != id || end_region.getOwner() != id) {
		throw "Not your region";
	}
	if (start_region.getBuilding().getName() != "MilitaryFactory") {
		throw "No military factory";
	}
	Config& configer = Config::getInstance();
	switch (operation.getOp())
	{
	case Operator::ProductArmy:
		int cost = configer.getConfig({ "Army","cost" }).get<int>() * operation.getSize();
		if (gold < cost) {
			throw "Not enough gold";
		}
		gold -= cost;
		end_region.addArmy(operation.getSize());
		break;
	case Operator::ProductWeapon0:
		std::vector<int> cost0 = configer.getConfig({ "Weapon", "0", "cost" }).template get<std::vector<int>>();
		if (gold < cost0[0] || oil < cost0[1] || electricity < cost0[2] || steel < cost0[3] || labor_limit - ocupied_labor < cost0[4]) {
			throw "Not enough resource";
		}
		gold -= cost0[0];
		oil -= cost0[1];
		electricity -= cost0[2];
		steel -= cost0[3];	
		end_region.addWeapon(0);
		break;
	case Operator::ProductWeapon1:
		std::vector<int> cost1 = configer.getConfig({ "Weapon", "1", "cost" }).template get<std::vector<int>>();
		if (gold < cost1[0] || oil < cost1[1] || electricity < cost1[2] || steel < cost1[3] || labor_limit - ocupied_labor < cost1[4]) {
			throw "Not enough resource";
		}
		gold -= cost1[0];
		oil -= cost1[1];
		electricity -= cost1[2];
		steel -= cost1[3];
		end_region.addWeapon(0);
		break;
	case Operator::ProductWeapon2:
		std::vector<int> cost2 = configer.getConfig({ "Weapon", "2", "cost" }).template get<std::vector<int>>();
		if (gold < cost2[0] || oil < cost2[1] || electricity < cost2[2] || steel < cost2[3] || labor_limit - ocupied_labor < cost2[4]) {
			throw "Not enough resource";
		}
		gold -= cost2[0];
		oil -= cost2[1];
		electricity -= cost2[2];
		steel -= cost2[3];
		end_region.addWeapon(0);
		break;
	}
}

void Player:: update(GlobalTimer& timer){
	std::vector<int> delta_resource = { 0,0,0,0,0 };
	double delta_t = timer.get_elapsed_time();
	regionmanager.calculate_delta_resources(delta_resource, delta_t, id);
	gold += delta_resource[0];
	oil += delta_resource[1];
	steel += delta_resource[2];
	electricity += delta_resource[3];
	labor_limit = delta_resource[4];
}
