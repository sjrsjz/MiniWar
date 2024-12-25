#include <queue>
#include <stdexcept>
#include <vector>
#include <exception>
#include <tuple>
#include <random>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <functional>
#include <thread>
#include <unordered_map>
#include "../../header/debug.h"
#include "../../header/Logic/RegionManager.h"
#include "../../header/utils/Config.h"
#include "../../header/Logic/Player.h"
#include "../../header/utils/Point.h"

template<typename Key, typename Value>
using umap = std::unordered_map<Key, Value>;
using json = nlohmann::json;
#define INF 1 << 30

class AITimer {
	private:
		using Clock = std::chrono::high_resolution_clock;
		using TimePoint = std::chrono::time_point<Clock>;

		TimePoint start_time;
		TimePoint pause_time;
		double accumulated_time; // 已累积的时间（秒）
		bool is_paused;

	public:
		AITimer() : accumulated_time(0.0), is_paused(false) {
			start_time = Clock::now();
		}

		void reset() {
			start_time = Clock::now();
			accumulated_time = 0.0;
			is_paused = false;
		}

		void pause() {
			if (!is_paused) {
				pause_time = Clock::now();
				accumulated_time += std::chrono::duration<double>(pause_time - start_time).count();
				is_paused = true;
			}
		}

		void resume() {
			if (is_paused) {
				start_time = Clock::now();
				is_paused = false;
			}
		}

		double elapsedSeconds() const {
			if (is_paused) {
				return accumulated_time;
			} else {
				auto now = Clock::now();
				return accumulated_time + std::chrono::duration<double>(now - start_time).count();
			}
		}
};

class AI {
	int gold;
	std::vector<int> weapons;
	std::vector<int> arm_level;	
	int maxLevel = 2;
	std::tuple<int, int> capital;
	RegionManager& regionManager = RegionManager::getInstance();
	Config& config = Config::getInstance();
	int id;
	json increaseParameter;
	std::tuple<int, int> playerCapital;
	float size;
	int regionSize;
	int playerRegionSize;
	double A = 10000000.0;
	double k = 0.01;
	double t0 = 50;
	AITimer Timer;
	bool canMove = true;
	bool canDefend = true;
	double delta_t = 0;
	double last_t = 0;
	std::vector<std::tuple<int, int>> AIRegions;
	std::vector<std::tuple<int, int>> playerRegions;
	std::vector<std::tuple<int, int>> border;
	std::vector<std::pair<std::tuple<int, int>, int>> distance;
	double averageForce;
	double playerAverageForce;
	json weaponCost;
	std::list<std::tuple<int, int>> isAttacked;

	const double formula(double t) {
		return 0.1 * regionSize * A / (1 + exp(-k * (t - t0)));
	}

public:
	AI(){}

	void create()
	{
		// temp
		// TODO
		weaponCost = config.getConfig({"ResearchInstitution", "OUpLevelCost"});
		gold = 1000;
		weapons = { 0, 0, 0 };
		arm_level = { 1, 0, 0, 0 };
		id = 1;
		try {
			Player& player = regionManager.get_player();
			int playerCapitalX = player.get_capital_x();
			int playerCapitalY = player.get_capital_y();
			playerCapital = std::make_tuple(playerCapitalX, playerCapitalY);
		} catch (std::exception& e) {
			throw new std::runtime_error("Player Capital not found");
		}

		std::tuple<float, float> originSize = config.getConfig({"Region", "OriginSize"}).template get<std::tuple<float, float>>();


		int mapWidth = regionManager.get_map_width();
		int mapHeight = regionManager.get_map_height();
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> disX(3, mapWidth - 3);
		std::uniform_int_distribution<int> disY(3, mapHeight - 3);
		std::uniform_int_distribution<int> disSize(std::get<0>(originSize), std::get<1>(originSize));
		size = disSize(gen);
		while (true){
			int x = disX(gen);
			int y = disY(gen);
			bool flag = true;
			for (int i = -3; i <= 3; i++)
			{
				for (int j = -3; j <= 3; j++)
				{
					if (i*i + j*j > size*size) continue;
					try {
						if (regionManager.get_region(x + i, y + j).getOwner() != -1)
						{
							flag = false;
							break;
						}
					} catch (std::exception& e) {
						continue;
					}
					
				}
			}
			if (flag) {
				capital = std::make_tuple(x, y);
				break;
			}
		}

		for (int i = -3; i <= 3; i++)
		{
			for (int j = -3; j<= 3; j++)
			{
				if (i*i + j*j > size*size) continue;
				try {
					regionManager.get_region(std::get<0>(capital) + i, std::get<1>(capital) + j).setOwner(id);
				} catch (std::exception& e) {
					continue;
				}
			}
		}
		regionManager.get_region(std::get<0>(capital), std::get<1>(capital)).setMaxHp(1000);
		regionManager.get_region(std::get<0>(capital), std::get<1>(capital)).setHp(1000);
		regionManager.get_region(std::get<0>(capital), std::get<1>(capital)).getArmy().addArmy(200);

	}		

	void create(int id) {
		weaponCost = config.getConfig({"ResearchInstitution", "OUpLevelCost"});
		gold = 1000;
		weapons = { 0, 0, 0 };
		arm_level = { 1, 0, 0, 0 };
		this->id = id;
		try {
			Player& player = regionManager.get_player();
			int playerCapitalX = player.get_capital_x();
			int playerCapitalY = player.get_capital_y();
			playerCapital = std::make_tuple(playerCapitalX, playerCapitalY);
		} catch (std::exception e) {
			throw new std::runtime_error("Player Capital not found");
		}

		std::tuple<float, float> originSize = config.getConfig({"Region", "OriginSize"}).template get<std::tuple<float, float>>();


		int mapWidth = regionManager.get_map_width();
		int mapHeight = regionManager.get_map_height();
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> disX(3, mapWidth - 3);
		std::uniform_int_distribution<int> disY(3, mapHeight - 3);
		std::uniform_int_distribution<int> disSize(std::get<0>(originSize), std::get<1>(originSize));
		size = disSize(gen);
		while (true){
			int x = disX(gen);
			int y = disY(gen);
			bool flag = true;
			for (int i = -3; i <= 3; i++)
			{
				for (int j = -3; j <= 3; j++)
				{
					if (i * i + j * j > size*size) continue;
					try {
						if (regionManager.get_region(x + i, y + j).getOwner() != -1)
						{
							flag = false;
							break;
						}
					} catch (std::exception& e) {
						continue;
					}
				}
			}
			if (flag) {
				capital = std::make_tuple(x, y);
				break;
			}
		}

		for (int i = -3; i <= 3; i++)
		{
			for (int j = -3; j<= 3; j++)
			{
				if (i*i + j*j > size*size) continue;
				try {
					regionManager.get_region(std::get<0>(capital) + i, std::get<1>(capital) + j).setOwner(id);
				}catch(std::exception& e) {
					continue;
				}
			}
		}
		regionManager.get_region(std::get<0>(capital), std::get<1>(capital)).setMaxHp(1000);
		regionManager.get_region(std::get<0>(capital), std::get<1>(capital)).setHp(1000);
		regionManager.get_region(std::get<0>(capital), std::get<1>(capital)).getArmy().addArmy(200);
	}		

	void setParameter(int id) {
		// TODO
		std::string difficulty;
		if (id == 1) {
			difficulty = "Easy";
		} else if (id == 2) {
			difficulty = "Normal";
		} else if (id == 3) {
			difficulty = "Hard";
		}
		const json parameter = config.getConfig({"AIParameter", difficulty});
		this->A = parameter["A"].template get<double>();
		this->k = parameter["k"].template get<double>();
		this->t0 = parameter["t0"].template get<double>();
	}

	void attack() {
		// TODO
		for (int i = 0; i < weapons.size(); i++) {
			if (weapons[i] == 0) continue;
			for (auto playerRegion: playerRegions) {
				for (auto aiRegion: AIRegions) {
					Point start(std::get<0>(aiRegion), std::get<1>(aiRegion));
					Point end(std::get<0>(playerRegion), std::get<1>(playerRegion));
					auto attackRange = regionManager.get_weapon(i).getAttackRange();
					double dist = start.distance(end) / regionManager.get_map_width();
					auto [min, max] = attackRange;
					int damage = regionManager.get_weapon(i).getDamage(arm_level[i + 1]);
					double time = start.distance(end) / regionManager.get_weapon(i).getAttackSpeed(arm_level[i + 1]);
					if (dist >= min && dist <= max) {
						regionManager.attack_region_missle(i, arm_level[i + 1], start, end, time);
					}
				}
			}
		}
	}

	void defend() {
		// TODO
		if (canMove && canDefend) {
			moveArmy();
		}
	}

	void timeInit() {
		AIRegions.clear();
		playerRegions.clear();
		border.clear();
		distance.clear();
		averageForce = 0;
		playerAverageForce = 0;
		for (int i = 0; i < regionManager.get_map_width(); i++) {
			for (int j = 0; j < regionManager.get_map_height(); j++) {
				if (regionManager.get_region(i, j).getOwner() == id) {
					AIRegions.push_back(std::make_tuple(i, j));
				} else if (regionManager.get_region(i, j).getOwner() == 0) {
					playerRegions.push_back(std::make_tuple(i, j));
				} 
			}
		}
		for (int i = 0; i < regionManager.get_map_width(); i++) {
			for (int j = 0; j < regionManager.get_map_height(); j++) {
				if (isBorder(i, j)) {
					border.push_back(std::make_tuple(i, j));
				}
			}
		}
		this->regionSize = AIRegions.size();
		this->playerRegionSize = playerRegions.size();

		for (auto AIRegion : AIRegions) {
			int distance = 0;
			for (auto playerRegion : playerRegions) {
				auto [x1, y1] = AIRegion;
				auto [x2, y2] = playerRegion;
				distance += (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
			}
			this->distance.push_back(std::make_pair(AIRegion, distance));
		}
		std::sort(distance.begin(), distance.end(), [](const std::pair<std::tuple<int, int>, int>& a, const std::pair<std::tuple<int, int>, int>& b) {
				return a.second < b.second;
				});

		for (auto AIRegion : AIRegions) {
			auto [x, y] = AIRegion;
			Region& region = regionManager.get_region(x, y);
			Army& regionArmy = region.getArmy();
			averageForce += regionArmy.getForce();
		}

		averageForce /= regionSize;

		for (auto playerRegion : playerRegions) {
			auto [x, y] = playerRegion;
			Region& region = regionManager.get_region(x, y);
			Army& regionArmy = region.getArmy();
			playerAverageForce += regionArmy.getForce();
		}

		
		for (const auto& AIRegion : AIRegions) {
			auto [AIx, AIy] = AIRegion;
			for (auto it = isAttacked.begin(); it != isAttacked.end(); ) {
				auto [x, y] = *it;

				if (x == AIx && y == AIy) {
					it = isAttacked.erase(it);
				} else {
					++it; 
				}
			}
		}

		playerAverageForce /= playerRegionSize;
	}

	void increase() {
		// TODO
		timeInit();		
		delta_t += Timer.elapsedSeconds() - last_t;
		last_t = Timer.elapsedSeconds();
		if (delta_t < 1) return;

		DEBUG::DebugOutput("gold: ", this->gold);
		DEBUG::DebugOutput("army: ", this->averageForce);
		DEBUG::DebugOutput("canMove: ", this->canMove);
		DEBUG::DebugOutput("canDefend: ", this->canDefend);
		this->gold += formula(Timer.elapsedSeconds());	
		delta_t -= 1;
		int buildArmy = std::min((int)(this->gold * 0.3), 8000);
		//json ArmyInfo = config.getConfig({"Army"});
		int cost = 1000;// ArmyInfo["cost"].template get<int>();
		Army& army = regionManager.get_region(std::get<0>(capital), std::get<1>(capital)).getArmy();
		army.addArmy(buildArmy / cost);
		this->gold -= buildArmy;

		int biuldWeapon = (int)this->gold * 0.5;
		double dist = std::sqrt(this->distance[0].second) / playerRegionSize / regionManager.get_map_width();
		if (dist <= 0.25) {
			if (biuldWeapon >= regionManager.get_weapon(0).getAICost() && arm_level[1] > 0) {
				weapons[0]++;
				this->gold -= regionManager.get_weapon(0).getAICost();
			}
		} else if (dist <= 0.5) {
			if (biuldWeapon >= regionManager.get_weapon(1).getAICost() && arm_level[2] > 0) {
				weapons[1]++;
				this->gold -= regionManager.get_weapon(1).getAICost();
			}
		} else {
			if (biuldWeapon >= regionManager.get_weapon(2).getAICost() && arm_level[3] > 0) {
				weapons[2]++;
				this->gold -= regionManager.get_weapon(2).getAICost();
			}
		}
		int maxLevelCount = 0;
		for (auto level : arm_level) {
			if (level == maxLevel) {
				maxLevelCount++;
			}
		}

		if (maxLevelCount == 4) {
			return;
		}
		int buildLevel = (int)this->gold * 0.2;
		int armycost = INF;
		if (arm_level[0] < maxLevel + 1)
			armycost = weaponCost["Army"].template get<std::vector<int>>()[arm_level[0] - 1];
		if (buildLevel >= armycost && arm_level[0] < maxLevel) {
			arm_level[0]++;
			this->gold -= armycost;
		}
		int weapon1cost = INF;
		int weapon2cost = INF;
		int weapon3cost = INF;
		if (arm_level[1] < maxLevel)
			weapon1cost = weaponCost["0"].template get<std::vector<int>>()[arm_level[1]];
		if (arm_level[2] < maxLevel)
			weapon2cost = weaponCost["1"].template get<std::vector<int>>()[arm_level[2]];
		if (arm_level[3] < maxLevel)
			weapon3cost = weaponCost["2"].template get<std::vector<int>>()[arm_level[3]];
		if (dist <= 0.25) {
			if (buildLevel >= weapon1cost && arm_level[1] < maxLevel) {
				arm_level[1]++;
				this->gold -= weapon1cost;
			}
		} else if (dist <= 0.5) {
			if (buildLevel >= weapon2cost && arm_level[2] < maxLevel) {
				arm_level[2]++;
				this->gold -= weapon2cost;
			}
		} else {
			if (buildLevel >= weapon3cost && arm_level[3] < maxLevel) {
				arm_level[3]++;
				this->gold -= weapon3cost;
			}
		}
	}

	bool isBorder(int x, int y) {
		if (regionManager.get_region(x, y).getOwner() == id) return false;
		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				if (x + i < 0 || x + i >= regionManager.get_map_width() || y + j < 0 || y + j >= regionManager.get_map_height()) continue;
				if (regionManager.get_region(x + i, y + j).getOwner() == id && !(i==0 && j==0)) {
					return true;
				}
			}
		}

		return false;
	}

	void sleep(double seconds) {
		this->canMove = false;
		auto start = std::chrono::steady_clock::now();
		std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
		auto end = std::chrono::steady_clock::now();
		DEBUG::DebugOutput( "Elapsed time: ", std::chrono::duration<double>(end - start).count());
		this->canMove = true;
	}

	struct Transaction {
		std::tuple<int, int> from;
		std::tuple<int, int> to;
		int force;
	};

	void moveArmy() {
		std::vector<std::pair<std::tuple<int, int>, int>> army;
		std::vector<std::pair<std::tuple<int, int>, int>> originForce;
		for (auto region : distance) {
			army.push_back(std::make_pair(region.first, 0));	
			Army& tmp = regionManager.get_region(std::get<0>(region.first), std::get<1>(region.first)).getArmy();
			originForce.push_back(std::make_pair(region.first, tmp.getForce()));
		}
		std::vector<double> weights(regionSize);
		int sumArmy = 0;
		int sumDistance = 0;
		for (auto AIRegion : AIRegions) {
			auto [x, y] = AIRegion;
			Region& region = regionManager.get_region(x, y);
			Army& regionArmy = region.getArmy();
			sumArmy += regionArmy.getForce();
		}
		for (auto item : this->distance) {
			sumDistance += item.second;
		}
		for (int i = 0; i < regionSize; i++) {
			weights[i] = 1.0 / this->distance[i].second;
		}

		float sumWeight = 0;
		for (auto i : weights) {
			sumWeight += i;
		}
		for (auto& weight : weights) {
			weight /= sumWeight;
		}
		int target = 0;
		for (int i = 0; i < regionSize; i++){
			army[i].second = std::floor(sumArmy * weights[i]);
			target += army[i].second;
		}

		int remain = sumArmy - target; // ??
		remain = remain >= army.size() ? army.size() - 1 : remain;
		for (int i = 0; i < remain; i++) {
			army[i].second++;
		}
		std::vector<std::pair<std::tuple<int, int>, int>> diff;

		for (int i = 0; i < regionSize; i++) {
			diff.push_back(std::make_pair(army[i].first, army[i].second - originForce[i].second));
		}

		std::queue<int> surplus, deficit;
		std::vector<Transaction> transactions;

		for (int i = 0; i < regionSize; i++) {
			if (diff[i].second > 0) {
				surplus.push(i);
			} else if (diff[i].second < 0) {
				deficit.push(i);
			}
		}

		while (!surplus.empty() && !deficit.empty()) {
			int s = surplus.front();
			int d = deficit.front();
			surplus.pop();
			deficit.pop();
			int force = std::min(diff[s].second, -diff[d].second);
			diff[s].second -= force;
			diff[d].second += force;
			transactions.push_back({army[s].first, army[d].first, force});
			if (diff[s].second > 0) {
				surplus.push(s);
			} else if (diff[s].second < 0) {
				deficit.push(s);
			}
			if (diff[d].second > 0) {
				surplus.push(d);
			} else if (diff[d].second < 0) {
				deficit.push(d);
			}
		}

		double maxTime = 0;

		for (auto transaction : transactions) {
			auto [from, to, force] = transaction;
			Point start = Point(std::get<0>(from), std::get<1>(from));
			Point end = Point(std::get<0>(to), std::get<1>(to));
			int armyLevel = this->arm_level[0];
			DEBUG::DebugOutput("moveArmy() calls move_Army");
			DEBUG::DebugOutput("from (", start.getX(), ",", start.getY(), ")", "to", "(", end.getX(), ",", end.getY(), ")");
			maxTime = std::max(maxTime, regionManager.move_army(end, start, force, armyLevel));
		}

		std::thread t([this, maxTime](){
				this->sleep(std::ceil(maxTime));
				});
		t.detach();
		DEBUG::DebugOutput("maxTime: ", maxTime);
	}

	void armyAttack(Point start, int amount, Point end) {
		for (auto item : isAttacked) {
			if (std::get<0>(item) == end.getX() && std::get<1>(item) == end.getY()) {
				return;
			}
		}
		this->canMove = false;
		this->canDefend = true;
		DEBUG::DebugOutput("armyAttack() called");
		DEBUG::DebugOutput("from (", start.getX(), ",", start.getY(), ")", "to", "(", end.getX(), ",", end.getY(), ")");
		Army& army = regionManager.get_region(start.getX(), start.getY()).getArmy();
		int curForce = army.getForce() * 0.7;
		if (army.getForce() * 0.7 >= amount) {
			DEBUG::DebugOutput("armyAttack only one region attack");
			double time = regionManager.move_army(start, end, amount, this->arm_level[0]);
			this->isAttacked.emplace_back(std::make_tuple(end.getX(), end.getY()));
			this->canMove = true;
			DEBUG::DebugOutput("armyAttack only one region attack need time", time);
			return;
		}
		std::vector<Point> regionlist;
		for (int i = -1; i <= 1 ;i++) {
			for (int j = -1; j <= 1; j++) {
				try {
					if (regionManager.get_region(start.getX() + i, start.getY() + j).getOwner() != id) continue;
					Army& tmp = regionManager.get_region(start.getX() + i, start.getY() + j).getArmy();
					curForce +=	tmp.getForce() * 0.7;
					regionlist.push_back(Point(start.getX() + i, start.getY() + j));
					double maxTime = 0;
					int armyLevel = this->arm_level[0];
					if (curForce >= amount) {
						for (auto region : regionlist) {
							DEBUG::DebugOutput("armyAttack() calls move_army()");
							Army& t = regionManager.get_region(region.getX(), region.getY()).getArmy();
							maxTime = std::max(maxTime, regionManager.move_army(region, start, t.getForce() * 0.7, armyLevel));
							DEBUG::DebugOutput("move_army() finished");
						}
						this->isAttacked.emplace_back(std::make_tuple(end.getX(), end.getY()));
						DEBUG::DebugOutput("ArmyAttack time: ", maxTime);
						this->canMove = false;
						std::this_thread::sleep_for(std::chrono::milliseconds((int)(maxTime * 1100)));
						DEBUG::DebugOutput("ArmyAttack finished: ");
						this->canMove = true;
						regionManager.move_army(start, end, amount, armyLevel);
						return;
					}
				} catch(std::exception& e) {
					continue;
				}
				
			}
		}
		this->canMove = true;
	}


	void expand() {
		// TODO
		if (!canMove) {
			canDefend = false;
			return;
		}
		canDefend = true;
		std::vector<std::pair<std::tuple<int, int>, int>> borderDistance;
		for (auto borderRegion : border) {
			int distance = 0;
			for (auto AIRegion : AIRegions) {
				auto [x1, y1] = borderRegion;
				auto [x2, y2] = AIRegion;
				distance += (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
			}
			borderDistance.push_back(std::make_pair(borderRegion, distance));
		}
		std::sort(borderDistance.begin(), borderDistance.end(), [](const std::pair<std::tuple<int, int>, int>& a, const std::pair<std::tuple<int, int>, int>& b) {
				return a.second < b.second;
				});


		if (regionSize < playerRegionSize && averageForce < playerAverageForce) {
			// TODO
			for (int i = regionSize - 1; i >= regionSize / 2; i--) {
				auto [x, y] = borderDistance[i].first;
				Army& borderArmy = regionManager.get_region(x, y).getArmy();
				int borderArmyForce = borderArmy.getForce(); 
				if (borderArmyForce < averageForce - 20) {
					Point maxForce(0, 0);
					int maxForceValue = 0;
					for (int j = -1; j <= 1; j++) {
						for (int k = -1; k <= 1; k++) {
							if (x + j < 0 || x + j >= regionManager.get_map_width() || y + k < 0 || y + k >= regionManager.get_map_height()) continue;
							if (regionManager.get_region(x + i, y + j).getOwner() == id) {
								Army& tmp = regionManager.get_region(x + j, y + k).getArmy();
								if (tmp.getForce() >= maxForceValue) {
									maxForceValue = tmp.getForce();
									maxForce = Point(x + j, y + k);
								}
							}
						}
					}	
					bool flag = false;
					for (auto item : isAttacked) {
						auto [Ax, Ay] = item;
						if (Ax == x && Ay == y) {
							flag = true;
							break;
						}
					}
					if (flag) {
						continue;
					}
					if (maxForce.getX() == 0 && maxForce.getY() == 0) {
						continue;
					}
					std::thread t([this, maxForce, borderArmyForce, x, y](){
							this->armyAttack(maxForce, borderArmyForce + 1, Point(x, y));
							});
					this->canMove = false;
					t.detach();
					break;
				}
			}
		} else {
			// TODO
			for (int i = 0; i <= regionSize / 2; i++) {
				auto [x, y] = borderDistance[i].first;
				Army& borderArmy = regionManager.get_region(x, y).getArmy();
				int borderArmyForce = borderArmy.getForce(); 
				if (borderArmyForce < averageForce - 20) {
					Point maxForce(0, 0);
					int maxForceValue = 0;
					for (int j = -1; j <= 1; j++) {
						for (int k = -1; k <= 1; k++) {
							int w = regionManager.get_map_width();
							int h = regionManager.get_map_height();
							if (x + j < 0 || x + j >= regionManager.get_map_width() || y + k < 0 || y + k >= regionManager.get_map_height()) continue;
							if (regionManager.get_region(x + j, y + k).getOwner() == id) {
								Army& tmp = regionManager.get_region(x + j, y + k).getArmy();
								if (tmp.getForce() >= maxForceValue) {
									maxForceValue = tmp.getForce();
									maxForce = Point(x + j, y + k);
								}
							}
						}
					}
					bool flag = false;
					for (auto item : isAttacked) {
						auto [Ax, Ay] = item;
						if (Ax == x && Ay == y) {
							flag = true;
							break;
						}
					}
					if (flag) {
						continue;
					}
					if (maxForce.getX() == 0 && maxForce.getY() == 0) {
						continue;
					}
					std::thread t([this, maxForce, borderArmyForce, x, y](){
							this->armyAttack(maxForce, borderArmyForce + 1, Point(x, y));
							});
					this->canMove = false;
					t.detach();
					break;
				}
			}
		}
	}

	void update(bool isPause = false) {
		if (isPause) {
			Timer.pause();
			return;
		} else {
			Timer.resume();
		}


		//DEBUG::DebugOutput("AI source", this->gold);
		//DEBUG::DebugOutput("canMove: ", this->canMove);
		//DEBUG::DebugOutput("AI Called increse()");
		
		this->increase();
		//DEBUG::DebugOutput("AI Called defend()");
		this->defend();
		//DEBUG::DebugOutput("AI Called expand()");
		this->expand();
		//DEBUG::DebugOutput("AI Called attack()");
		this->attack();
	}
	
};
