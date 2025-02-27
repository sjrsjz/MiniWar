﻿#include <queue>
#include <mutex>
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
#include <list>
#include "../../header/debug.h"
#include "../../header/Logic/RegionManager.h"
#include "../../header/utils/Config.h"
#include "../../header/Logic/Player.h"
#include "../../header/utils/Point.h"
void push_game_effects(GameEffect effect);

#define INF std::numeric_limits<int>::infinity()

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
	RegionManager& regionManager = RegionManager::instance_of();
	Config& config = Config::instance_of();
	int id;
	std::tuple<int, int> playerCapital;
	double size;
	int regionSize;
	int playerRegionSize;
	double A;
	double k = 0.01;
	double t0 = 150;
	AITimer Timer;
	double attackTime = 0;
	double last_at = 0;
	bool canRun = true;
	bool canMove = true;
	bool canDefend = true;
	bool capitalAlive = true;
	bool canAttack = true;
	double delta_t = 0;
	double last_t = 0;
	std::vector<std::tuple<int, int>> AIRegions;
	std::vector<std::tuple<int, int>> playerRegions;
	std::vector<std::tuple<int, int>> border;
	std::vector<std::pair<std::tuple<int, int>, int>> distance;
	double averageForce;
	double playerAverageForce;
	std::list<std::tuple<int, int>> isAttacked;

	const double formula(double t) {
		return 0.1 * regionSize * A / (1 + exp(-k * (t - t0)));
	}

public:
	AI(){}

	void clear() {
		gold = 0;
		weapons.clear();
		arm_level.clear();
		capital = std::make_tuple(0, 0);
		id = -1;
		AIRegions.clear();
		playerRegions.clear();
		border.clear();
		distance.clear();
		averageForce = 0;
		playerAverageForce = 0;
		size = 0;
		regionSize = 0;
		playerRegionSize = 0;
		isAttacked.clear();
	}

	void create()
	{
		// temp
		// TODO
		clear();
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
			throw std::runtime_error("Player Capital not found");
		}

		std::tuple<double, double> originSize = config.get_default_region_setting().OriginSize;


		int mapWidth = regionManager.map_width();
		int mapHeight = regionManager.map_height();
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
						if (regionManager.region(x + i, y + j).get_owner() != -1)
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
					regionManager.region(std::get<0>(capital) + i, std::get<1>(capital) + j).set_owner(id);
				} catch (std::exception& e) {
					continue;
				}
			}
		}
		regionManager.region(std::get<0>(capital), std::get<1>(capital)).set_max_HP(1000);
		regionManager.region(std::get<0>(capital), std::get<1>(capital)).set_HP(1000);
		regionManager.region(std::get<0>(capital), std::get<1>(capital)).get_army().add_amount(200);

	}		

	void create(int id) {
		clear();
		gold = 1000;
		weapons = { 0, 0, 0 };
		arm_level = { 1, 0, 0, 0 };
		this->id = id;
		try {
			Player& player = regionManager.get_player();
			int playerCapitalX = player.get_capital_x();
			int playerCapitalY = player.get_capital_y();
			playerCapital = std::make_tuple(playerCapitalX, playerCapitalY);
		} catch (std::exception& e) {
			throw std::runtime_error("Player Capital not found");
		}

		std::tuple<double, double> originSize = config.get_default_region_setting().OriginSize;


		int mapWidth = regionManager.map_width();
		int mapHeight = regionManager.map_height();
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
						if (regionManager.region(x + i, y + j).get_owner() != -1)
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
					regionManager.region(std::get<0>(capital) + i, std::get<1>(capital) + j).set_owner(id);
				} catch (std::exception& e) {
					continue;
				}
			}
		}
		regionManager.region(std::get<0>(capital), std::get<1>(capital)).set_max_HP(1000);
		regionManager.region(std::get<0>(capital), std::get<1>(capital)).set_HP(1000);
		regionManager.region(std::get<0>(capital), std::get<1>(capital)).get_army().add_amount(200);

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
		this->A = config.get_AI_parameter(difficulty).A;
		this->k = config.get_AI_parameter(difficulty).k;
		this->t0 = config.get_AI_parameter(difficulty).t0;
	}

	void attack() {
		// TODO
		if (!canAttack) {
			return;
		}
		int maxcnt = 20;
		int cnt = 0;
		int playerSize = playerRegions.size();
		int aiSize = AIRegions.size();
		int roundSize = std::ceil((double)maxcnt / playerSize);
		//std::cout << "A:" << this->A << std::endl;
		for (int i = 0; i < weapons.size(); i++) {
			if (weapons[i] == 0) continue;
			int roundCnt = 0;
			for (auto playerRegion: playerRegions) {
				for (auto aiRegion: AIRegions) {
					Point start(std::get<0>(aiRegion), std::get<1>(aiRegion));
					Point end(std::get<0>(playerRegion), std::get<1>(playerRegion));
					auto attackRange = regionManager.get_weapon(i).get_attack_range();
					double dist = start.distance(end) / regionManager.map_width();
					auto [min, max] = attackRange;
					int damage = regionManager.get_weapon(i).get_damage(arm_level[i + 1]);
					double time = start.distance(end) / regionManager.get_weapon(i).get_attack_speed(arm_level[i + 1]);
					if (dist >= min && dist <= max) {
						//DEBUGOUTPUT("WeaponAttack() called");
						regionManager.attack_region_missle(i, arm_level[i + 1], start, end, time);
						push_game_effects(GameEffect::GAME_EFFECT_PLAY_NUCLEAR_WARNING);
						cnt++;
						canAttack = false;
						//DEBUGOUTPUT("WeaponAttack() finished");
					}
					if (cnt >= maxcnt) {
						return;
					}
					roundCnt++;
					if (roundCnt >= roundSize) {
						break;
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
		for (int i = 0; i < regionManager.map_width(); i++) {
			for (int j = 0; j < regionManager.map_height(); j++) {
				if (regionManager.region(i, j).get_owner() == id) {
					AIRegions.push_back(std::make_tuple(i, j));
				} else if (regionManager.region(i, j).get_owner() != id && regionManager.region(i, j).get_owner() != -1) {
					playerRegions.push_back(std::make_tuple(i, j));
				} 
			}
		}
		for (int i = 0; i < regionManager.map_width(); i++) {
			for (int j = 0; j < regionManager.map_height(); j++) {
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
			Region& region = regionManager.region(x, y);
			Army& regionArmy = region.get_army();
			averageForce += regionArmy.get_force();
		}

		for (auto AIRegion : AIRegions) {
			auto [x, y] = AIRegion;
			auto [capitalX, capitalY] = capital;
			if (x == capitalX && y == capitalY) {
				capitalAlive = true;
				break;
			}
			capitalAlive = false;
		}

		if (!capitalAlive) {
			int SIZE = AIRegions.size();
			int mid = SIZE / 2;
			capital = AIRegions[mid];
			capitalAlive = true;
			auto [x, y] = capital;
		}

		averageForce /= regionSize;

		for (auto playerRegion : playerRegions) {
			auto [x, y] = playerRegion;
			Region& region = regionManager.region(x, y);
			Army& regionArmy = region.get_army();
			playerAverageForce += regionArmy.get_force();
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
		attackTime += Timer.elapsedSeconds() - last_at;	
		last_t = Timer.elapsedSeconds();
		last_at = Timer.elapsedSeconds();
		if (delta_t < 1) {
			canRun = false;
			return;
		}
		canRun = true;
		if (attackTime >= 30) {
			attackTime = 0;
			canAttack = true;
		}

		//DEBUGOUTPUT("gold: ", this->gold);
		//DEBUGOUTPUT("army: ", this->averageForce);
		//DEBUGOUTPUT("canMove: ", this->canMove);
		//DEBUGOUTPUT("canDefend: ", this->canDefend);
		this->gold += formula(Timer.elapsedSeconds());	
		delta_t -= 1;
		if (capitalAlive) {
			int buildArmy = std::min((int)(this->gold * 0.2), 2000);
			//int cost = 1000;// ArmyInfo["cost"].template get<int>();
			int cost = config.get_army_parameter().cost;
			Army& army = regionManager.region(std::get<0>(capital), std::get<1>(capital)).get_army();
			army.add_amount(buildArmy / cost);
			this->gold -= buildArmy;
		}

		int biuldWeapon = (int)this->gold * 0.9;
		int sumDis = 0;
		for (int i = 0; i < distance.size(); i++) {
			sumDis += distance[i].second;
		}
		double dist = std::sqrt(sumDis) / playerRegionSize / regionManager.map_width();
		if (dist <= 0.25) {
			if (biuldWeapon >= regionManager.get_weapon(0).get_AI_cost() && arm_level[1] > 0) {
				weapons[0]++;
				this->gold -= regionManager.get_weapon(0).get_AI_cost();
			}
		} else if (dist <= 0.5) {
			if (biuldWeapon >= regionManager.get_weapon(1).get_AI_cost() && arm_level[2] > 0) {
				weapons[1]++;
				this->gold -= regionManager.get_weapon(1).get_AI_cost();
			}
		} else {
			if (biuldWeapon >= regionManager.get_weapon(2).get_AI_cost() && arm_level[3] > 0) {
				weapons[2]++;
				this->gold -= regionManager.get_weapon(2).get_AI_cost();
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
		int buildLevel = (int)this->gold * 1;
		int armycost = INF;
		if (arm_level[0] < maxLevel + 1)
			//armycost = config.getArmy;
		if (buildLevel >= armycost && arm_level[0] < maxLevel) {
			arm_level[0]++;
			this->gold -= armycost;
		}
		int weapon1cost = INF;
		int weapon2cost = INF;
		int weapon3cost = INF;
		if (arm_level[1] <= maxLevel)
			//weapon1cost = weaponCost["0"].template get<std::vector<int>>()[arm_level[1]];
		if (arm_level[2] <= maxLevel)
			//weapon2cost = weaponCost["1"].template get<std::vector<int>>()[arm_level[2]];
		if (arm_level[3] <= maxLevel)
			//weapon3cost = weaponCost["2"].template get<std::vector<int>>()[arm_level[3]];
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
		if (regionManager.region(x, y).get_owner() == id) return false;
		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				if (x + i < 0 || x + i >= regionManager.map_width() || y + j < 0 || y + j >= regionManager.map_height()) continue;
				if (regionManager.region(x + i, y + j).get_owner() == id && !(i==0 && j==0)) {
					return true;
				}
			}
		}

		return false;
	}

	void sleep(double seconds) {
		auto start = std::chrono::steady_clock::now();
		std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
		auto end = std::chrono::steady_clock::now();		
		//DEBUGOUTPUT( "Elapsed time: ", std::chrono::duration<double>(end - start).count());
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
			Army& tmp = regionManager.region(std::get<0>(region.first), std::get<1>(region.first)).get_army();
			originForce.push_back(std::make_pair(region.first, tmp.get_force()));
		}
		std::vector<double> weights(regionSize);
		int sumArmy = 0;
		int sumDistance = 0;
		for (auto AIRegion : AIRegions) {
			auto [x, y] = AIRegion;
			Region& region = regionManager.region(x, y);
			Army& regionArmy = region.get_army();
			sumArmy += regionArmy.get_force();
		}
		for (auto item : this->distance) {
			sumDistance += item.second;
		}
		for (int i = 0; i < regionSize; i++) {
			weights[i] = 1.0 / this->distance[i].second;
		}

		double sumWeight = 0;
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

		double maxTime = -1;

		for (auto transaction : transactions) {
			auto [from, to, force] = transaction;
			Point start = Point(std::get<0>(from), std::get<1>(from));
			Point end = Point(std::get<0>(to), std::get<1>(to));
			int armyLevel = this->arm_level[0];
			//DEBUGOUTPUT("moveArmy() calls move_Army");
			//DEBUGOUTPUT("from (", start.x, ",", start.y, ")", "to", "(", end.x, ",", end.y, ")");
			maxTime = std::max(maxTime, regionManager.move_army(end, start, force, armyLevel));
		}
		if (maxTime == -1) {
			return;
		}

		this->canMove = false;
		std::thread t([this, maxTime](){
				this->sleep(std::ceil(maxTime));
				});
		t.detach();
		//DEBUGOUTPUT("maxTime: ", maxTime);
	}

	void armyAttack(Point start, int amount, Point end) {
		for (auto item : isAttacked) {
			if (std::get<0>(item) == end.x && std::get<1>(item) == end.y) {
				return;
			}
		}
		this->canMove = false;
		this->canDefend = true;
		//DEBUGOUTPUT("armyAttack() called");
		//DEBUGOUTPUT("from (", start.x, ",", start.y, ")", "to", "(", end.x, ",", end.y, ")");
		Army& army = regionManager.region(start.x, start.y).get_army();
		int curForce = army.get_force() * 0.7;
		if (army.get_force() * 0.7 >= amount) {
			//DEBUGOUTPUT("armyAttack only one region attack");
			try {
				double time = regionManager.move_army(start, end, curForce, this->arm_level[0]);
				this->isAttacked.emplace_back(std::make_tuple(end.x, end.y));
				this->canMove = true;
				return;
			}
			catch (std::exception& e) {
				DEBUGOUTPUT("armyAttack error: " + std::string(e.what()));
				this->canMove = true;
				return;
			}
			//DEBUGOUTPUT("armyAttack only one region attack need time", time);
		}
		std::vector<Point> regionlist;
		for (int i = -1; i <= 1 ;i++) {
			for (int j = -1; j <= 1; j++) {
				try {
					if (regionManager.region(start.x + i, start.y + j).get_owner() != id) continue;
					Army& tmp = regionManager.region(start.x + i, start.y + j).get_army();
					curForce +=	tmp.get_force() * 0.7;
					regionlist.push_back(Point(start.x + i, start.y + j));
					double maxTime = 0;
					int armyLevel = this->arm_level[0];
					if (curForce >= amount) {
						for (auto region : regionlist) {
							//DEBUGOUTPUT("armyAttack() calls move_army()");
							Army& t = regionManager.region(region.x, region.y).get_army();
							maxTime = std::max(maxTime, regionManager.move_army(region, start, t.get_force() * 0.7, armyLevel));
							//DEBUGOUTPUT("move_army() finished");
						}
						this->isAttacked.emplace_back(std::make_tuple(end.x, end.y));
						//DEBUGOUTPUT("ArmyAttack time: ", maxTime);
						this->canMove = false;
						std::this_thread::sleep_for(std::chrono::milliseconds((int)(maxTime * 1100)));
						//DEBUGOUTPUT("ArmyAttack finished: ");
						this->canMove = true;
						regionManager.move_army(start, end, curForce, armyLevel);
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
			for (int i = borderDistance.size() - 1; i >= borderDistance.size() / 2; i--) {
				auto [x, y] = borderDistance[i].first;
				Army& borderArmy = regionManager.region(x, y).get_army();
				int borderArmyForce = borderArmy.get_force(); 
				if (borderArmyForce < averageForce - 20) {
					Point maxForce(0, 0);
					int maxForceValue = 0;
					for (int j = -1; j <= 1; j++) {
						for (int k = -1; k <= 1; k++) {
							if (x + j < 0 || x + j >= regionManager.map_width() || y + k < 0 || y + k >= regionManager.map_height()) continue;
							if (regionManager.region(x + j, y + k).get_owner() == id) {
								Army& tmp = regionManager.region(x + j, y + k).get_army();
								if (tmp.get_force() >= maxForceValue) {
									maxForceValue = tmp.get_force();
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
					if (maxForce.x == 0 && maxForce.y == 0) {
						continue;
					}

					this->canMove = false;
					std::thread t([this, maxForce, borderArmyForce, x, y](){
							this->armyAttack(maxForce, borderArmyForce + 1, Point(x, y));
							});
					t.detach();
					break;
				}
			}
		} else {
			// TODO
			for (int i = 0; i <= borderDistance.size() / 2; i++) {
				auto [x, y] = borderDistance[i].first;
				Army& borderArmy = regionManager.region(x, y).get_army();
				int borderArmyForce = borderArmy.get_force(); 
				if (borderArmyForce < averageForce - 20) {
					Point maxForce(0, 0);
					int maxForceValue = 0;
					for (int j = -1; j <= 1; j++) {
						for (int k = -1; k <= 1; k++) {
							int w = regionManager.map_width();
							int h = regionManager.map_height();
							if (x + j < 0 || x + j >= regionManager.map_width() || y + k < 0 || y + k >= regionManager.map_height()) continue;
							if (regionManager.region(x + j, y + k).get_owner() == id) {
								Army& tmp = regionManager.region(x + j, y + k).get_army();
								if (tmp.get_force() >= maxForceValue) {
									maxForceValue = tmp.get_force();
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
					if (maxForce.x == 0 && maxForce.y == 0) {
						continue;
					}
					this->canMove = false;
					std::thread t([this, maxForce, borderArmyForce, x, y](){
							this->armyAttack(maxForce, borderArmyForce + 1, Point(x, y));
							});
					t.detach();
					break;
				}
			}
		}
	}

	void update(bool& isPause, bool& aiState) {


		if (isPause) {
			Timer.pause();
			return;
		}
		else {
			Timer.resume();
		}


		//DEBUGOUTPUT("AI source", this->gold);
		//DEBUGOUTPUT("canMove: ", this->canMove);
		//DEBUGOUTPUT("AI Called increse()");
		
		try {
			this->increase();
			if (regionSize == 0) {
				aiState = false;
				return;
			}
			//DEBUGOUTPUT("AI Called defend()");
			this->defend();
			//DEBUGOUTPUT("AI Called expand()");
			this->expand();
			//DEBUGOUTPUT("AI Called attack()");
			this->attack();
		}
		catch (std::exception e) {
			
		}
	}
	
};
