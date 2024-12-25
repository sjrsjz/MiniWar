#include "../../header/Logic/RegionManager.h"
#include "../../header/debug.h"
#include "../../header/Logic/Player.h"
#include <vector>
#include <random>


RegionManager::RegionManager() {
}

RegionManager::RegionManager(int width, int height){
	set(width, height);
}

RegionManager::~RegionManager() {
}

struct Node {
    Point pt;        // ��ǰ�ڵ�λ��
    int g_cost;      // ����㵽��ǰ��Ĵ���
    int h_cost;      // ���ʽ����ֵ�������پ��룩
    int f_cost;      // �ܴ��� f = g + h
    Node* parent;    // ���ڵ�ָ��

    Node(Point pt_, int g, int h, Node* parent_ = nullptr)
        : pt(pt_), g_cost(g), h_cost(h), f_cost(g + h), parent(parent_) {}

    // �Ƚ����ȶ��е�˳��f_cost С������
    bool operator>(const Node& other) const {
        return f_cost > other.f_cost;
    }
};

int manhattan_distance(Point a,Point b) {
    return std::abs(a.getX() - b.getX()) + std::abs(a.getY() - b.getY());
}

static std::vector<Point> reconstruct_path(Node* end_node) {
    std::vector<Point> path;
    for (Node* current = end_node; current != nullptr; current = current->parent) {
        path.push_back(current->pt);
    }
    std::reverse(path.begin(), path.end());
    return path;
}


std::vector<Point> astar(std::vector<std::vector<int>>& grid, Point start, Point end) {
    int rows = grid.size();
    int cols = grid[0].size();

    // �˸��������ڵ㣩
    std::vector<std::pair<int, int>> directions = {
        {0, 1}, {1, 0}, {0, -1}, {-1, 0},  // �ϡ��ҡ��¡���
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1} // ���ϡ����ϡ����¡�����
    };

    // ���ȶ��У�С���ѣ�
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open_set;

    // �洢�Ѿ����ʹ��Ľڵ�
    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));

    // ����ʼ��
    Node* start_node = new Node(start, 0, manhattan_distance(start, end));
    open_set.push(*start_node);

    while (!open_set.empty()) {
        Node current = open_set.top();
        open_set.pop();

        if (visited[current.pt.getX()][current.pt.getY()]) {
            continue;
        }

        visited[current.pt.getX()][current.pt.getY()] = true;

        // ����Ƿ񵽴��յ�
        if (current.pt.getX() == end.getX() && current.pt.getY() == end.getY()) {
            return reconstruct_path(&current);
        }

        // �������ڵ�
        for (const auto& dir : directions) {
            int nx = current.pt.getX() + dir.first;
            int ny = current.pt.getY() + dir.second;

            // ���߽���Ƿ����
            if (nx >= 0 && ny >= 0 && nx < rows && ny < cols && grid[nx][ny] == 1 && !visited[nx][ny]) {
                int move_cost = (dir.first == 0 || dir.second == 0) ? 10 : 14;
                Node* neighbor = new Node(Point(nx, ny), current.g_cost + move_cost, manhattan_distance(Point(nx, ny), end), new Node(current));
                open_set.push(*neighbor);
            }
        }
    }

    // ����Ҳ���·�������ؿ�
    return {};
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




	int id = get_region(start_x, start_y).getOwner();

	double distance = 0.f;

	Array<Region>& regions = get_regions();
	std::vector<std::vector<int>> player_region_matrix(regions.get_width(), std::vector<int>(regions.get_height(), 0));
	for (int i = 0; i < regions.get_width(); i++) {
		for (int j = 0; j < regions.get_height(); j++) {
			if (regions(i, j).getOwner() == id || regions(i,j).getOwner() == -1 || (i == end_x&&j==end_y)) {
				player_region_matrix[i][j] = 1;
			}
		}
	}


	std::vector<Point> path_points = astar(player_region_matrix, start, end);

	if (path_points.empty()) {
		return -1.f;
	}

	for (int i = 0; i < path_points.size(); i++) {
		path.push_back(std::make_tuple(path_points[i].getX(), path_points[i].getY()));
		if (i == 0) continue;
		distance += path_points[i].distance(path_points[i - 1]);
	}

	return distance;

	//int temp_x = start_x;
	//int temp_y = start_y;

	//static int NOT_AVALIABLE = 1000000;
	//static int straight_cost = 10;
	//static int diagonal_cost = 14;

	//int cost_list[8] = {};//0: left up 1: up 2: right up 3: left 4: right 5: left down 6: down 7: right down
	//bool up = true;
	//bool down = true;
	//bool left = true;
	//bool right = true;

	//while (!(temp_x != end_x && temp_y != end_y))
	//{
	//	path.push_back(std::make_tuple(temp_x, temp_y));

	//	if (temp_x - 1 < 0)
	//	{
	//		cost_list[0] = NOT_AVALIABLE;
	//		cost_list[3] = NOT_AVALIABLE;
	//		cost_list[5] = NOT_AVALIABLE;
	//		left = false;
	//	}
	//	if (temp_x + 1 > regions.get_width())
	//	{
	//		cost_list[2] = NOT_AVALIABLE;
	//		cost_list[4] = NOT_AVALIABLE;
	//		cost_list[7] = NOT_AVALIABLE;
	//		right = false;
	//	}
	//	if (temp_y - 1 < 0)
	//	{
	//		cost_list[5] = NOT_AVALIABLE;
	//		cost_list[6] = NOT_AVALIABLE;
	//		cost_list[7] = NOT_AVALIABLE;
	//		down = false;
	//	}
	//	if (temp_y + 1 > regions.get_height())
	//	{
	//		cost_list[0] = NOT_AVALIABLE;
	//		cost_list[1] = NOT_AVALIABLE;
	//		cost_list[2] = NOT_AVALIABLE;
	//		up = false;
	//	}
	//	if (up) {
	//		if (player_region_matrix[temp_x + (temp_y + 1) * regions.get_width()] == 1) {
	//			cost_list[0] = straight_cost * (1 + end_x - temp_x + end_y - temp_y);
	//		}
	//		else {
	//			cost_list[0] = NOT_AVALIABLE;
	//		}
	//		if (left) {
	//			if (player_region_matrix[temp_x - 1 + (temp_y + 1) * regions.get_width()] == 1) {
	//				cost_list[1] = diagonal_cost + straight_cost * (end_x - temp_x + end_y - temp_y);
	//			}
	//			else {
	//				cost_list[1] = NOT_AVALIABLE;
	//			}
	//		}
	//		if (right) {
	//			if (player_region_matrix[temp_x + 1 + (temp_y + 1) * regions.get_width()] == 1) {
	//				cost_list[2] = diagonal_cost + straight_cost * (end_x - temp_x + end_y - temp_y);
	//			}
	//			else {
	//				cost_list[2] = NOT_AVALIABLE;
	//			}
	//		}
	//	}
	//	if (down) {
	//		if (player_region_matrix[temp_x + (temp_y - 1) * regions.get_width()] == 1) {
	//			cost_list[5] = straight_cost * (1 + end_x - temp_x + end_y - temp_y);
	//		}
	//		else {
	//			cost_list[5] = NOT_AVALIABLE;
	//		}
	//		if (left) {
	//			if (player_region_matrix[temp_x - 1 + (temp_y - 1) * regions.get_width()] == 1) {
	//				cost_list[6] = diagonal_cost + straight_cost * (end_x - temp_x + end_y - temp_y);
	//			}
	//			else {
	//				cost_list[6] = NOT_AVALIABLE;
	//			}
	//		}
	//		if (right) {
	//			if (player_region_matrix[temp_x + 1 + (temp_y - 1) * regions.get_width()] == 1) {
	//				cost_list[7] = diagonal_cost + straight_cost * (end_x - temp_x + end_y - temp_y);
	//			}
	//			else {
	//				cost_list[7] = NOT_AVALIABLE;
	//			}
	//		}
	//	}
	//	if (left) {
	//		if (player_region_matrix[temp_x - 1 + temp_y * regions.get_width()] == 1) {
	//			cost_list[3] = straight_cost * (1 + end_x - temp_x + end_y - temp_y);
	//		}
	//		else {
	//			cost_list[3] = NOT_AVALIABLE;
	//		}
	//	}
	//	if (right) {
	//		if (player_region_matrix[temp_x + 1 + temp_y * regions.get_width()] == 1) {
	//			cost_list[4] = straight_cost * (1 + end_x - temp_x + end_y - temp_y);
	//		}
	//		else {
	//			cost_list[4] = NOT_AVALIABLE;
	//		}
	//	}

	//	int min_cost = NOT_AVALIABLE, min_way = 0;
	//	for (int i = 0; i < 8; i++) {
	//		if (cost_list[i] < min_cost) {
	//			min_cost = cost_list[i];
	//			min_way = i;
	//		}
	//	}
	//	if (min_cost == NOT_AVALIABLE) {
	//		return -1.f;
	//	}

	//	switch (min_way) {
	//	case 0:
	//		temp_y++;
	//		distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x, temp_y - 1));
	//		break;
	//	case 1:
	//		temp_x--;
	//		temp_y++;
	//		distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x + 1, temp_y - 1));
	//		break;
	//	case 2:
	//		temp_x++;
	//		temp_y++;
	//		distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x - 1, temp_y - 1));
	//		break;
	//	case 3:
	//		temp_x--;
	//		distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x + 1, temp_y));
	//		break;
	//	case 4:
	//		temp_x++;
	//		distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x - 1, temp_y));
	//		break;
	//	case 5:
	//		temp_y--;
	//		distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x, temp_y + 1));
	//		break;
	//	case 6:
	//		temp_x--;
	//		temp_y--;
	//		distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x + 1, temp_y + 1));
	//		break;
	//	case 7:
	//		temp_x++;
	//		temp_y--;
	//		distance += calculate_Euclidean_distance(std::make_tuple(temp_x, temp_y), std::make_tuple(temp_x - 1, temp_y + 1));
	//		break;
	//	}
	//	player_region_matrix[temp_x + temp_y * regions.get_width()] = 0;
	//}
	//return distance;
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

	regions(0, 0).setOwner(0);	
	regions(0, 1).setOwner(0);
	regions(0, 2).setOwner(0);
	regions(1, 0).setOwner(0);
	regions(1, 1).setOwner(0);
	regions(1, 2).setOwner(0);
	regions(2, 0).setOwner(0);
	regions(2, 1).setOwner(0);
	regions(2, 2).setOwner(0);
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
	army.time = current_time + time;
	army.path = path;
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
	missle.time = current_time + time;
	missle.start_point = std::make_tuple(start.getX(), start.getY());
	missle.end_point = std::make_tuple(end.getX(), end.getY());
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

static RegionManager instance;
RegionManager& RegionManager::getInstance() {
	return instance;
}
void RegionManager::clear_building(Region& region) {
	region.getBuilding().remove();
}

void RegionManager::update(GlobalTimer& timer) {
	current_time += timer.get_elapsed_time();
	while (!moving_armies.empty() && moving_armies.top().time <= current_time) {
		DEBUG::DebugOutput("Moving army: ", moving_armies.top().amount);
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

	while (!moving_missles.empty() && moving_missles.top().time <= current_time) {
		DEBUG::DebugOutput("Moving missle: ", moving_missles.top().weapon_id);
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
