#include "../../header/Logic/Player.h"
#include "../../header/Logic/RegionManager.h"

struct Node{
    int x, y;
    float cost;
    float heuristic;
    Node* parent;

    Node(int x, int y, float cost, float heuristic, Node* parent = nullptr)
    : x(x), y(y), cost(cost), heuristic(heuristic), parent(parent){}

    float total_cost() const{
        return cost + heuristic;
    }

    bool operator>(const Node& other) const{
        return total_cost() > other.total_cost();
    }
};

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
    delete &regionmanager;
}

float Player::calculate_distance(Point start, Point end){
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

    auto heuristic = [&](int x, int y){
        return std::sqrt(std::pow(x-end_x, 2) + std::pow(y-end_y, 2));
    };

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open_list;
    std::unordered_map<int, Node*> all_nodes;
    open_list.emplace(start_x, start_y, 0, heuristic(start_x, start_y));

    while (!open_list.empty()){
        Node current = open_list.top();
        open_list.pop();

        if(current.x == end_x && current.y == end_y){
            float distance = 0;
            while(current.parent != nullptr){
                distance += regions(current.x, current.y).getArmy().getSpeed();
                current = *current.parent;
            }
            return distance;
        }

        std::vector<std::pair<int,int>> neibours = {
            {current.x-1, current.y},
            {current.x+1, current.y},
            {current.x, current.y-1},
            {current.x, current.y+1}
        };

        for(auto& neibour : neibours){
            int nx = neibour.first;
            int ny = neibour.second;
            if(nx >= 0 && nx < regions.get_width() && ny >= 0 && ny < regions.get_height() && player_region_martix[nx + ny * regions.get_width()] == 1){
                float new_cost = current.cost + std::sqrt(std::pow(nx - current.x, 2) + std::pow(ny - current.y, 2));
                Node* neighbor_node = new Node(nx, ny, new_cost, heuristic(nx, ny), &current);
                open_list.push(*neighbor_node);
                all_nodes[nx + ny * regions.get_width()] = neighbor_node;
            }
        }
    } 

    return -1;
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
    float distance = calculate_distance(start, end);
    if (distance == -1) {
        throw "No such road";
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
