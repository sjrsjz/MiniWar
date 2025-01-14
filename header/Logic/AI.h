#include <queue>
#include <cmath>
#include <mutex>
#include <stdexcept>
#include <type_traits>
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
#include <climits>
#include "../../header/debug.h"
#include "../../header/Logic/RegionManager.h"
#include "../../header/utils/Config.h"
#include "../../header/Logic/Player.h"
#include "../../header/utils/Point.h"
#include "./GameEffect.h"
#include "../utils/GlobalTimer.h"
#include "Weapon.h"

using json = nlohmann::json;

namespace DISTANCE {
	constexpr double SHORT = 0.25;
	constexpr double MID = 0.5;
	constexpr double LONG = 0.75;
};

enum WEAPON {
	WEAPON1,
	WEAPON2,
	WEAPON3
};

const int ARMY = 0;

void push_game_effects(GameEffect effect);

class Time {
    GlobalTimer& Timer = GlobalTimer::instance_of();
    double last_t = Timer.get_acc_time();
    double delta_t = 0;
    double target;
    bool* state;  // 改为指针
    bool is_bound = false;
    std::function<void()> call_back_func;

public:
    Time(double target) : target(target) {
        state = new bool(false);
    }
    
    Time(double target, bool& state_ref) : target(target) {
        state = &state_ref;
        *state = false;
        is_bound = true;
    }
    
    Time(double target, bool& state_ref, std::function<void()> call_back_func) 
        : target(target), call_back_func(call_back_func) {
        state = &state_ref;
        *state = false;
        is_bound = true;
    }
    
    Time() : target(1) {
        state = new bool(false);
    }

    bool update() {
        delta_t += Timer.get_acc_time() - last_t;
        last_t = Timer.get_acc_time();
        if (delta_t < target) {
            *state = false;
            return false;
        }
        delta_t -= target;
        *state = true;
        return true;
    }
    Time(const Time& other) : target(other.target), Timer(other.Timer) {
        if (other.is_bound) {
            state = other.state;
            is_bound = true;
        } else {
            state = new bool(*other.state);
            is_bound = false;
        }
        call_back_func = other.call_back_func;
        last_t = other.last_t;
        delta_t = other.delta_t;
    }

    // 添加赋值运算符
    Time& operator=(const Time& other) {
        if (this != &other) {
            if (!is_bound && state != nullptr) {
                delete state;
            }
            
            target = other.target;
            if (other.is_bound) {
                state = other.state;
                is_bound = true;
            } else {
                state = new bool(*other.state);
                is_bound = false;
            }
            call_back_func = other.call_back_func;
            last_t = other.last_t;
            delta_t = other.delta_t;
        }
        return *this;
    }
    
    ~Time() {
        if (call_back_func) {
            call_back_func();
        }
        if (!is_bound && state != nullptr) {
            delete state;  // 正确删除动态分配的内存
			state = nullptr;
        }
    }
};



// 玩家指玩家和其他AI
class AI {
	int gold;
	std::vector<int> weapons;
	std::vector<int> arm_level;	// 0: army, 1: weapon1, 2: weapon2, 3: weapon3
	std::vector<double> army_uplevel_cost;
	std::vector<std::vector<double>> weapon_uplevel_cost;
	int maxLevel = 2; //武器最高等级
	std::tuple<int, int> capital; //首都坐标
	RegionManager& regionManager = RegionManager::instance_of();
	Config& config = Config::instance_of();
	int id;
	/* json increaseParameter; */
	std::tuple<int, int> playerCapital; //玩家首都坐标
	int curAIRegionSize; //AI领地数量
	int playerRegionSize; //玩家领地数量
	double A; //AI每秒资源增长上限
	double k = 0.01; //AI资源增长速度
	double t0 = 150; //AI资源增速达到A的一半所需时间
	/* AITimer Timer; */
	std::list<Time> times; //移动中的时间(代替thread)
	GlobalTimer& Timer = GlobalTimer::instance_of(); // 全局计时器
	double attackTime = 0; //上一次导弹攻击累计时间
	double last_at = 0; //上一个游戏帧导弹攻击累计时间
	bool canRun = true; //资源每秒增加, 判断是否经过一秒
	bool canMove = true; //是否可以移动军队
	bool canDefend = true; //是否可以防御
	bool capitalAlive = true; //AI首都是否存活
	bool canAttack = true; //是否可以用导弹攻击
	/* double delta_t = 0; //资源增长时间间隔 */
	/* double last_t = 0; //上一个游戏帧资源增长时间 */
	std::vector<std::tuple<int, int>> AIRegions; //AI领地坐标
	std::vector<std::tuple<int, int>> playerRegions; //玩家领地坐标
	std::vector<std::tuple<int, int>> border; //AI边界坐标
	std::vector<std::pair<std::tuple<int, int>, int>> distance; //AI领地到玩家领地距离,所有距离的平方和,从小到大排序
	double averageForce; //AI平均兵力
	double playerAverageForce; //玩家平均兵力
	/* json weaponCost{}; //升级兵力和武器的花费 */
	std::list<std::tuple<int, int>> isAttacked; // 已经被攻击的领地坐标

	// 资源增长公式
	const double formula(double t) {
		return 0.1 * curAIRegionSize * A / (1 + exp(-k * (t - t0)));
	}

public:
	AI(){}


	void create(int id = 1) {
		clear();
		/* weaponCost = config.getConfig({"ResearchInstitution", "OUpLevelCost"}); */
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
		int originMaxRange = 3; // 生成领地的范围
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> disX(originMaxRange, mapWidth - originMaxRange);
		std::uniform_int_distribution<int> disY(originMaxRange, mapHeight - originMaxRange);
		std::uniform_int_distribution<int> disSize(std::get<0>(originSize), std::get<1>(originSize));
		double size = disSize(gen);
		int surroundSize = 3; //检测周围3格内是否有其他的领地
		int maxLoopCount = 30; // 最大循环次数
		while (maxLoopCount--) {
			int x = disX(gen);
			int y = disY(gen);
			bool flag = true;
			for (int i = -surroundSize; i <= surroundSize; i++)
			{
				for (int j = -surroundSize; j <= surroundSize; j++)
				{
					if (i*i + j*j > size*size) continue;
						if (outOfRange(x + i, y + j)) continue;
						if (regionManager.region(x + i, y + j).get_owner() != -1)
						{
							flag = false;
							break;
						}
					
				}
			}
			if (flag) {
				capital = std::make_tuple(x, y);
				break;
			}
		}

		for (int i = -surroundSize; i <= surroundSize; i++)
		{
			for (int j = -surroundSize; j<= surroundSize; j++)
			{
				if (i*i + j*j > size*size) continue;
				if (outOfRange(std::get<0>(capital) + i, std::get<1>(capital) + j)) continue;
					regionManager.region(std::get<0>(capital) + i, std::get<1>(capital) + j).set_owner(id);
			}
		}
		regionManager.region(std::get<0>(capital), std::get<1>(capital)).set_max_HP(1000);
		regionManager.region(std::get<0>(capital), std::get<1>(capital)).set_HP(1000);
		regionManager.region(std::get<0>(capital), std::get<1>(capital)).get_army().add_amount(200);

		}		

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
		curAIRegionSize = 0;
		playerRegionSize = 0;
		/* weaponCost.clear(); */
		isAttacked.clear();
	}
	
	void update(char& aiState) {
		//DEBUG::DebugOutput("AI source", this->gold);
		//DEBUG::DebugOutput("canMove: ", this->canMove);
		//DEBUG::DebugOutput("AI Called increse()");
		
		try {
			this->increase();
			if (curAIRegionSize == 0) {
				aiState = false;
				return;
			}
			//DEBUG::DebugOutput("AI Called defend()");
			this->defend();
			//DEBUG::DebugOutput("AI Called expand()");
			this->expand();
			//DEBUG::DebugOutput("AI Called attack()");
			this->attack();
		}
		catch (std::exception e) {
			
		}
	}

	// 根据难度设置AI参数
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
		this->army_uplevel_cost = config.get_AI_parameter(difficulty).ArmyUpLevelCost;
		this->weapon_uplevel_cost = config.get_AI_parameter(difficulty).WeaponUpLevelCost;
	}

private:
	// 检测是否越界
	bool outOfRange(int x, int y) {
		return x < 0 || x >= regionManager.map_width() || y < 0 || y >= regionManager.map_height();
	}



	void attack() {
		// TODO
		if (!canAttack) {
			return;
		}
		int maxcnt = 20; //最大导弹攻击次数
		int cnt = 0; //当前导弹攻击次数
		/* int playerSize = playerRegions.size(); */

		int roundSize = std::ceil((double)maxcnt / playerRegionSize);
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
						DEBUG::DebugOutput("WeaponAttack() called");
						std::cout << "roundSize: " << roundSize << std::endl;
						regionManager.attack_region_missle(i, arm_level[i + 1], start, end, time);
						std::cout << "Weapon attack" << std::endl;
						push_game_effects(GameEffect::GAME_EFFECT_PLAY_NUCLEAR_WARNING);
						cnt++;
						canAttack = false;
						DEBUG::DebugOutput("WeaponAttack() finished");
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
		this->curAIRegionSize = AIRegions.size();
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
			std::cout << "capital position" << x << " " << y << std::endl;
		}

		averageForce /= curAIRegionSize;

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
	
	Time srcIncreaseTime{1}; //资源增长时间间隔
	Time attackIncreaseTime{30}; //导弹攻击时间间隔

	void increase() {
		// TODO
		timeInit();		

		for (auto it = times.begin(); it != times.end(); ) {
			if (it->update()) {
				it = times.erase(it);
			} else {
				++it;
			}
		}

		/* delta_t += Timer.elapsedSeconds() - last_t; */
		/* attackTime += Timer.elapsedSeconds() - last_at; */	
		/* last_t = Timer.elapsedSeconds(); */
		/* last_at = Timer.elapsedSeconds(); */
		if (!srcIncreaseTime.update()) {
			return;
		}
		if (attackIncreaseTime.update()) {
			canAttack = true;
		}

		/* DEBUG::DebugOutput("gold: ", this->gold); */
		/* DEBUG::DebugOutput("army: ", this->averageForce); */
		/* DEBUG::DebugOutput("canMove: ", this->canMove); */
		/* DEBUG::DebugOutput("canDefend: ", this->canDefend); */
		this->gold += formula(Timer.get_acc_time());	
		int buildArmy = std::min((int)(this->gold * 0.2), 2000);
		/* json ArmyInfo = config.getConfig({"Army"}); */
		//int cost = 1000;// ArmyInfo["cost"].template get<int>();
		int cost = config.get_army_parameter().cost;
		Army& army = regionManager.region(std::get<0>(capital), std::get<1>(capital)).get_army();
		army.add_amount(buildArmy / cost);
		this->gold -= buildArmy;

		int biuldWeapon = (int)this->gold * 0.9;
		int sumDis = 0;
		for (int i = 0; i < distance.size(); i++) {
			sumDis += distance[i].second;
		}
		double dist = std::sqrt(sumDis) / playerRegionSize / regionManager.map_width();
		if (dist <= DISTANCE::SHORT) {
			if (biuldWeapon >= config.get_weapon_parameter(WEAPON::WEAPON1).AICost && arm_level[WEAPON::WEAPON1 + 1] > 0) {
				weapons[WEAPON::WEAPON1]++;
				this->gold -= config.get_weapon_parameter(WEAPON::WEAPON1).AICost;
			}
		} else if (dist <= DISTANCE::MID) {
			if (biuldWeapon >= config.get_weapon_parameter(WEAPON::WEAPON2).AICost && arm_level[WEAPON::WEAPON2 + 1] > 0) {
				weapons[WEAPON::WEAPON2]++;
				this->gold -= config.get_weapon_parameter(WEAPON::WEAPON2).AICost;
			}
		} else {
			if (biuldWeapon >= config.get_weapon_parameter(WEAPON::WEAPON3).AICost && arm_level[WEAPON::WEAPON3] > 0) {
				weapons[WEAPON::WEAPON3]++;
				this->gold -= config.get_weapon_parameter(WEAPON::WEAPON3).AICost;
			}
		}
		int maxLevelCount = 0;
		for (auto level : arm_level) {
			if (level == maxLevel) {
				maxLevelCount++;
			}
		}

		if (maxLevelCount == weapons.size()) {
			return;
		}
		int buildLevel = (int)this->gold * 1;
		int armycost = INT_MAX;
		if (arm_level[ARMY] < maxLevel + 1)
			/* armycost = weaponCost["Army"].template get<std::vector<int>>()[arm_level[ARMY] - 1]; */
			armycost = army_uplevel_cost[arm_level[ARMY]];
		if (buildLevel >= armycost && arm_level[ARMY] < maxLevel) {
			arm_level[ARMY]++;
			this->gold -= armycost;
		}
		int weapon1cost = INT_MAX;
		int weapon2cost = INT_MAX;
		int weapon3cost = INT_MAX;
		if (arm_level[WEAPON::WEAPON1 + 1] <= maxLevel)
			/* weapon1cost = weaponCost["0"].template get<std::vector<int>>()[arm_level[WEAPON::WEAPON1 + 1]]; */
			weapon1cost = weapon_uplevel_cost[WEAPON::WEAPON1][arm_level[WEAPON::WEAPON1 + 1]];
		if (arm_level[WEAPON::WEAPON2 + 1] <= maxLevel)
			/* weapon2cost = weaponCost["1"].template get<std::vector<int>>()[arm_level[WEAPON::WEAPON2 + 1]]; */
			weapon2cost = weapon_uplevel_cost[WEAPON::WEAPON2][arm_level[WEAPON::WEAPON2 + 1]];
		if (arm_level[WEAPON::WEAPON3 + 1] <= maxLevel)
			/* weapon3cost = weaponCost["2"].template get<std::vector<int>>()[arm_level[WEAPON::WEAPON3 + 1]]; */
			weapon3cost = weapon_uplevel_cost[WEAPON::WEAPON3][arm_level[WEAPON::WEAPON3 + 1]];
		if (dist <= DISTANCE::SHORT) {
			if (buildLevel >= weapon1cost && arm_level[WEAPON::WEAPON1 + 1] < maxLevel) {
				arm_level[WEAPON::WEAPON1 + 1]++;
				this->gold -= weapon1cost;
			}
		} else if (dist <= DISTANCE::MID) {
			if (buildLevel >= weapon2cost && arm_level[WEAPON::WEAPON2 + 1] < maxLevel) {
				arm_level[WEAPON::WEAPON2 + 1]++;
				this->gold -= weapon2cost;
			}
		} else {
			if (buildLevel >= weapon3cost && arm_level[WEAPON::WEAPON3 + 1] < maxLevel) {
				arm_level[WEAPON::WEAPON3 + 1]++;
				this->gold -= weapon3cost;
			}
		}
	}

	bool isBorder(int x, int y) {
		int surroundSize = 1;
		if (regionManager.region(x, y).get_owner() == id) return false;
		for (int i = -surroundSize; i <= surroundSize; i++) {
			for (int j = -surroundSize; j <= surroundSize; j++) {
				if (x + i < 0 || x + i >= regionManager.map_width() || y + j < 0 || y + j >= regionManager.map_height()) continue;
				if (regionManager.region(x + i, y + j).get_owner() == id && !(i==0 && j==0)) {
					return true;
				}
			}
		}

		return false;
	}


	struct Transaction {
		std::tuple<int, int> from;
		std::tuple<int, int> to;
		int force;
	};

	void moveArmy() {
		std::vector<std::pair<std::tuple<int, int>, int>> assigned_army;
		std::vector<std::pair<std::tuple<int, int>, int>> originForce;
		for (auto region : distance) {
			assigned_army.push_back(std::make_pair(region.first, 0));	
			Army& tmp = regionManager.region(std::get<0>(region.first), std::get<1>(region.first)).get_army();
			originForce.push_back(std::make_pair(region.first, tmp.get_force()));
		}
		std::vector<double> weights(curAIRegionSize);
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
		for (int i = 0; i < curAIRegionSize; i++) {
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
		for (int i = 0; i < curAIRegionSize; i++){
			assigned_army[i].second = std::floor(sumArmy * weights[i]);
			target += assigned_army[i].second;
		}

		int remain = sumArmy - target; // ??
		remain = remain >= assigned_army.size() ? assigned_army.size() - 1 : remain;
		for (int i = 0; i < remain; i++) {
			assigned_army[i].second++;
		}
		std::vector<std::pair<std::tuple<int, int>, int>> diff;

		for (int i = 0; i < curAIRegionSize; i++) {
			diff.push_back(std::make_pair(assigned_army[i].first, assigned_army[i].second - originForce[i].second));
		}

		std::queue<int> surplus, deficit;
		std::vector<Transaction> transactions;

		for (int i = 0; i < curAIRegionSize; i++) {
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
			transactions.push_back({assigned_army[s].first, assigned_army[d].first, force});
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
			DEBUG::DebugOutput("moveArmy() calls move_Army");
			DEBUG::DebugOutput("from (", start.x, ",", start.y, ")", "to", "(", end.x, ",", end.y, ")");
			maxTime = std::max(maxTime, regionManager.move_army(end, start, force, armyLevel));
		}
		if (maxTime == -1) {
			return;
		}

		this->canMove = false;

		times.push_back(Time(maxTime, this->canMove));
		times.push_back(Time(maxTime, this->canDefend));

		/* std::thread t([this, maxTime](){ */
		/* 		this->sleep(std::ceil(maxTime)); */
		/* 		}); */
		/* t.detach(); */
		DEBUG::DebugOutput("maxTime: ", maxTime);
	}

	void armyAttack(Point start, int amount, Point end) {
		for (auto item : isAttacked) {
			if (std::get<0>(item) == end.x && std::get<1>(item) == end.y) {
				return;
			}
		}
		this->canMove = false;
		this->canDefend = true;
		DEBUG::DebugOutput("armyAttack() called");
		DEBUG::DebugOutput("from (", start.x, ",", start.y, ")", "to", "(", end.x, ",", end.y, ")");
		Army& army = regionManager.region(start.x, start.y).get_army();
		int curForce = army.get_force() * 0.7;
		if (army.get_force() * 0.7 >= amount) {
			DEBUG::DebugOutput("armyAttack only one region attack");
			double time = regionManager.move_army(start, end, curForce, this->arm_level[0]);
			this->isAttacked.emplace_back(std::make_tuple(end.x, end.y));
			this->canMove = true;
			DEBUG::DebugOutput("armyAttack only one region attack need time", time);
			return;
		}
		std::vector<Point> regionlist;
		int borderSize = 1; //边界范围
		for (int i = -borderSize; i <= borderSize ;i++) {
			for (int j = -borderSize; j <= borderSize; j++) {
				if (outOfRange(start.x + i, start.y + j)) continue;
				if (regionManager.region(start.x + i, start.y + j).get_owner() != id) continue;
				Army& tmp = regionManager.region(start.x + i, start.y + j).get_army();
				curForce +=	tmp.get_force() * 0.7;
				regionlist.push_back(Point(start.x + i, start.y + j));
				double maxTime = 0;
				int armyLevel = this->arm_level[0];
				if (curForce >= amount) {
					for (auto region : regionlist) {
						DEBUG::DebugOutput("armyAttack() calls move_army()");
						Army& t = regionManager.region(region.x, region.y).get_army();
						maxTime = std::max(maxTime, regionManager.move_army(region, start, t.get_force() * 0.7, armyLevel));
						DEBUG::DebugOutput("move_army() finished");
					}
					this->isAttacked.emplace_back(std::make_tuple(end.x, end.y));
					DEBUG::DebugOutput("ArmyAttack time: ", maxTime);
					this->canMove = false;
					/* std::this_thread::sleep_for(std::chrono::milliseconds((int)(maxTime * 1100))); */
					/* DEBUG::DebugOutput("ArmyAttack finished: "); */
					/* this->canMove = true; */
					times.push_back(Time(maxTime, this->canMove,[this, start, end, curForce, armyLevel](){
							this->regionManager.move_army(start, end, curForce, armyLevel);
							}));
					return;
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


		if (curAIRegionSize < playerRegionSize && averageForce < playerAverageForce) {
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
					this->armyAttack(maxForce, borderArmyForce + 1, Point(x, y));
					/* std::thread t([this, maxForce, borderArmyForce, x, y](){ */
					/* 		this->armyAttack(maxForce, borderArmyForce + 1, Point(x, y)); */
					/* 		}); */
					/* t.detach(); */
					/* break; */
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
					this->armyAttack(maxForce, borderArmyForce + 1, Point(x, y));
					/* std::thread t([this, maxForce, borderArmyForce, x, y](){ */
					/* 		this->armyAttack(maxForce, borderArmyForce + 1, Point(x, y)); */
					/* 		}); */
					/* t.detach(); */
					break;
				}
			}
		}
	}
	
};

class AI_groups{
	int count;
	std::vector<AI> groups;
	std::vector<char> is_alive;
	bool no_alive = false; 
public:
	AI_groups(int count) : count(count) {
		for (int i = 0; i < count; i++) {
			groups.push_back(AI());
		}
		is_alive.resize(count, true);
	}

	void update() {
		for (int i = 0; i < count; i++) {
			char& state = is_alive[i];
			groups[i].update(state);
		}
		for (int i = 0; i < count; i++) {
			if (is_alive[i]) {
				no_alive = false;
				break;
			}
		}
	}

	void clear() {
		for (int i = 0; i < count; i++) {
			groups[i].clear();
		}
	}

	void set_parameter(int id) {
		for (int i = 0; i < count; i++) {
			groups[i].create(i + 1);
			groups[i].setParameter(id);
		}
	}

	bool get_status() {
		return no_alive;
	}
};

