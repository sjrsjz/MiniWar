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




// 玩家指玩家和其他AI
class AI {
	
	class Timed_trigger {
		GlobalTimer& Timer = GlobalTimer::instance_of();
		double last_t = Timer.get_acc_time();
		double delta_t = 0;
		double target;
		bool* state;  // 改为指针
		bool is_bound = false;
		std::function<void()> call_back_func;

	public:
		Timed_trigger(double target) : target(target) {
			state = new bool(false);
		}
		
		Timed_trigger(double target, bool& state_ref) : target(target) {
			state = &state_ref;
			*state = false;
			is_bound = true;
		}
		
		Timed_trigger(double target, bool& state_ref, std::function<void()> call_back_func) 
			: target(target), call_back_func(call_back_func) {
			state = &state_ref;
			*state = false;
			is_bound = true;
		}
		
		Timed_trigger() : target(1) {
			state = new bool(false);
		}

		bool update() {
			if (delta_t < 0) delta_t = 0;
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
		Timed_trigger(const Timed_trigger& other) : target(other.target), Timer(other.Timer) {
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
		Timed_trigger& operator=(const Timed_trigger& other) {
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
		
		~Timed_trigger() {
			if (call_back_func) {
				try{
					call_back_func();
				} catch (std::exception& e) {
					DEBUGOUTPUT("Time call_back_func error: ", e.what());
				}
			}
			if (!is_bound && state != nullptr) {
				delete state;  // 正确删除动态分配的内存
				state = nullptr;
			}
		}
	};
	int gold;
	std::vector<int> weapons;
	std::vector<int> arm_level;	// 0: army, 1: weapon1, 2: weapon2, 3: weapon3
	std::vector<double> army_uplevel_cost;
	std::vector<std::vector<double>> weapon_uplevel_cost;
	int max_level = 2; //武器最高等级
	std::tuple<int, int> capital; //首都坐标
	RegionManager& region_manager = RegionManager::instance_of();
	Config& config = Config::instance_of();
	int id;
	/* json increaseParameter; */
	std::tuple<int, int> player_capital; //玩家首都坐标
	int cur_AI_region_size; //AI领地数量
	int player_region_size; //玩家领地数量
	double A = 300; //AI每秒资源增长上限
	double k = 0.01; //AI资源增长速度
	double t0 = 150; //AI资源增速达到A的一半所需时间
	/* AITimer Timer; */
	std::list<Timed_trigger> times; //移动中的时间(代替thread)
	GlobalTimer& Timer = GlobalTimer::instance_of(); // 全局计时器
	/* double attackTime = 0; //上一次导弹攻击累计时间 */
	/* double last_at = 0; //上一个游戏帧导弹攻击累计时间 */
	bool can_run = true; //资源每秒增加, 判断是否经过一秒
	bool can_move = true; //是否可以移动军队
	bool can_defend = true; //是否可以防御
	bool capital_alive = true; //AI首都是否存活
	bool can_attack = true; //是否可以用导弹攻击
	double army_increase_cost_weight = 0.2; //军队增长花费权重
	double max_army_increase_cost = 2000; //军队增长花费权重上限
	double weapon_increase_cost_weight = 0.9; //武器制造花费权重
	double uplevel_cost_weight = 1; //升级花费权重
	double expand_army_max_weight = 0.7; //扩张军队最大权重
	/* double delta_t = 0; //资源增长时间间隔 */
	/* double last_t = 0; //上一个游戏帧资源增长时间 */
	std::vector<std::tuple<int, int>> AI_regions; //AI领地坐标
	std::vector<std::tuple<int, int>> player_regions; //玩家领地坐标
	std::vector<std::tuple<int, int>> border; //AI边界坐标
	std::vector<std::pair<std::tuple<int, int>, int>> distance; //AI领地到玩家领地距离,所有距离的平方和,从小到大排序
	double average_force; //AI平均兵力
	double player_average_force; //玩家平均兵力
	/* json weaponCost{}; //升级兵力和武器的花费 */
	std::list<std::tuple<int, int>> is_attacked; // 已经被攻击的领地坐标

	// 资源增长公式
	const double formula(double t) {
		return sqrt(cur_AI_region_size) * A / (1 + exp(-k * (t - t0))) + 1500 / cur_AI_region_size;
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
			Player& player = region_manager.get_player();
			int player_capital_x = player.get_capital_x();
			int player_capital_y = player.get_capital_y();
			player_capital = std::make_tuple(player_capital_x, player_capital_y);
		} catch (std::exception& e) {
			throw std::runtime_error("Player Capital not found");
		}

		std::tuple<double, double> origin_size = config.get_default_region_setting().OriginSize;


		int map_width = region_manager.map_width();
		int map_height = region_manager.map_height();
		int origin_max_range = 3; // 生成领地的范围
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> dis_x(origin_max_range, map_width - origin_max_range);
		std::uniform_int_distribution<int> dis_y(origin_max_range, map_height - origin_max_range);
		std::uniform_int_distribution<int> dis_size(std::get<0>(origin_size), std::get<1>(origin_size));
		double size = dis_size(gen);
		int surround_size = 3; //检测周围3格内是否有其他的领地
		int max_loop_count = 30; // 最大循环次数
		while (max_loop_count--) {
			int x = dis_x(gen);
			int y = dis_y(gen);
			bool flag = true;
			for (int i = -surround_size; i <= surround_size; i++)
			{
				for (int j = -surround_size; j <= surround_size; j++)
				{
					if (i*i + j*j > size*size) continue;
						if (out_of_range(x + i, y + j)) continue;
						if (region_manager.region(x + i, y + j).get_owner() != -1)
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

		for (int i = -surround_size; i <= surround_size; i++)
		{
			for (int j = -surround_size; j<= surround_size; j++)
			{
				if (i*i + j*j > size*size) continue;
				if (out_of_range(std::get<0>(capital) + i, std::get<1>(capital) + j)) continue;
					region_manager.region(std::get<0>(capital) + i, std::get<1>(capital) + j).set_owner(id);
			}
		}
		region_manager.region(std::get<0>(capital), std::get<1>(capital)).set_max_HP(1000);
		region_manager.region(std::get<0>(capital), std::get<1>(capital)).set_HP(1000);
		region_manager.region(std::get<0>(capital), std::get<1>(capital)).get_army().add_amount(200);

		}		

	void clear() {
		gold = 0;
		weapons.clear();
		arm_level.clear();
		capital = std::make_tuple(0, 0);
		id = -1;
		AI_regions.clear();
		player_regions.clear();
		border.clear();
		distance.clear();
		average_force = 0;
		player_average_force = 0;
		cur_AI_region_size = 0;
		player_region_size = 0;
		/* weaponCost.clear(); */
		is_attacked.clear();
		times.clear();
	}
	
	void update(char& ai_state) {
		//DEBUGOUTPUT("canMove: ", this->canMove);
		//DEBUGOUTPUT("AI Called increse()");
		can_run = src_increase_time.update();
		if (!can_run) {
			return;
		}
		
		try {
			this->increase();
			if (cur_AI_region_size == 0) {
				ai_state = false;
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

	// 根据难度设置AI参数
	void set_parameter(int id) {
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
	bool out_of_range(int x, int y) {
		return x < 0 || x >= region_manager.map_width() || y < 0 || y >= region_manager.map_height();
	}



	void attack() {
		// TODO
		if (!can_attack) {
			return;
		}
		int maxcnt = 100; //最大导弹攻击次数
		int cnt = 0; //当前导弹攻击次数
		/* int playerSize = playerRegions.size(); */

		int round_size = std::ceil((double)maxcnt / player_region_size);
		//std::cout << "A:" << this->A << std::endl;
		for (int i = 0; i < weapons.size(); i++) {
			if (weapons[i] == 0) continue;
			int round_cnt = 0;
			for (auto player_region: player_regions) {
				for (auto aiRegion: AI_regions) {
					if (weapons[i] == 0) break;
					Point start(std::get<0>(aiRegion), std::get<1>(aiRegion));
					Point end(std::get<0>(player_region), std::get<1>(player_region));
					auto attackRange = region_manager.get_weapon(i).get_attack_range();
					double dist = start.distance(end) / region_manager.map_width();
					auto [min, max] = attackRange;
					int damage = region_manager.get_weapon(i).get_damage(arm_level[i + 1]);
					double time = start.distance(end) / region_manager.get_weapon(i).get_attack_speed(arm_level[i + 1]);
					if (dist >= min && dist <= max) {
						DEBUGOUTPUT("WeaponAttack() called");
						std::cout << "roundSize: " << round_size << std::endl;
						region_manager.attack_region_missle(i, arm_level[i + 1], start, end, time);
						weapons[i]--;
						std::cout << "Weapon attack" << std::endl;
						push_game_effects(GameEffect::GAME_EFFECT_PLAY_NUCLEAR_WARNING);
						cnt++;
						can_attack = false;
						DEBUGOUTPUT("WeaponAttack() finished");
					}
					if (cnt >= maxcnt) {
						return;
					}
					round_cnt++;
					if (round_cnt >= round_size) {
						break;
					}
				}
			}
		}
	}

	void defend() {
		// TODO
		if (can_move && can_defend) {
			move_army();
		}
	}

	void time_init() {
		AI_regions.clear();
		player_regions.clear();
		border.clear();
		distance.clear();
		// AI_regions.resize(0);
		// player_regions.resize(0);
		// border.resize(0);
		// distance.resize(0);
		average_force = 0;
		player_average_force = 0;
		for (int i = 0; i < region_manager.map_width(); i++) {
			for (int j = 0; j < region_manager.map_height(); j++) {
				if (region_manager.region(i, j).get_owner() == id) {
					AI_regions.push_back(std::make_tuple(i, j));
				} else if (region_manager.region(i, j).get_owner() != id && region_manager.region(i, j).get_owner() != -1) {
					player_regions.push_back(std::make_tuple(i, j));
				} 
			}
		}
		for (int i = 0; i < region_manager.map_width(); i++) {
			for (int j = 0; j < region_manager.map_height(); j++) {
				if (isBorder(i, j)) {
					border.push_back(std::make_tuple(i, j));
				}
			}
		}
		this->cur_AI_region_size = AI_regions.size();
		this->player_region_size = player_regions.size();

		for (auto AI_region : AI_regions) {
			int distance = 0;
			for (auto player_region : player_regions) {
				auto [x1, y1] = AI_region;
				auto [x2, y2] = player_region;
				distance += (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
			}
			this->distance.push_back(std::make_pair(AI_region, distance));
		}
		std::sort(distance.begin(), distance.end(), [](const std::pair<std::tuple<int, int>, int>& a, const std::pair<std::tuple<int, int>, int>& b) {
				return a.second < b.second;
				});

		for (auto AI_region : AI_regions) {
			auto [x, y] = AI_region;
			Region& region = region_manager.region(x, y);
			Army& region_army = region.get_army();
			average_force += region_army.get_force();
		}

		for (auto AI_region : AI_regions) {
			auto [x, y] = AI_region;
			auto [capital_x, capital_y] = capital;
			if (x == capital_x && y == capital_y) {
				capital_alive = true;
				break;
			}
			capital_alive = false;
		}

		if (!capital_alive) {
			int SIZE = AI_regions.size();
			int mid = SIZE / 2;
			capital = AI_regions[mid];
			capital_alive = true;
			auto [x, y] = capital;
			/* std::cout << "capital position" << x << " " << y << std::endl; */
		}

		average_force /= cur_AI_region_size;

		for (auto player_region : player_regions) {
			auto [x, y] = player_region;
			Region& region = region_manager.region(x, y);
			Army& region_army = region.get_army();
			player_average_force += region_army.get_force();
		}

		
		for (const auto& AI_region : AI_regions) {
			auto [AIx, AIy] = AI_region;
			for (auto it = is_attacked.begin(); it != is_attacked.end(); ) {
				auto [x, y] = *it;

				if (x == AIx && y == AIy) {
					it = is_attacked.erase(it);
				} else {
					++it; 
				}
			}
		}

		player_average_force /= player_region_size;
	}
	
	Timed_trigger src_increase_time{1}; //资源增长时间间隔
	Timed_trigger attack_increase_time{30}; //导弹攻击时间间隔

	void increase() {
		// TODO
		time_init();		

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
		// if (!src_increase_time.update()) {
		// 	return;
		// }
		DEBUGOUTPUT("AI source:", this->id, this->gold);
		if (attack_increase_time.update()) {
			can_attack = true;
		}

		/* DEBUGOUTPUT("gold: ", this->gold); */
		/* DEBUGOUTPUT("army: ", this->averageForce); */
		/* DEBUGOUTPUT("canMove: ", this->canMove); */
		/* DEBUGOUTPUT("canDefend: ", this->canDefend); */
		double increase = formula(Timer.get_acc_time());
		this->gold += increase;	
		DEBUGOUTPUT("AI source increase:", this->id, increase);
		int buildArmy = std::min((this->gold * army_increase_cost_weight), max_army_increase_cost);
		/* json ArmyInfo = config.getConfig({"Army"}); */
		//int cost = 1000;// ArmyInfo["cost"].template get<int>();
		int cost = config.get_army_parameter().cost;
		Army& army = region_manager.region(std::get<0>(capital), std::get<1>(capital)).get_army();
		army.add_amount(buildArmy / cost);
		this->gold -= buildArmy;

		int biuldWeapon = (int)this->gold * weapon_increase_cost_weight;
		int sumDis = 0;
		for (int i = 0; i < distance.size(); i++) {
			sumDis += distance[i].second;
		}
		double dist = std::sqrt(sumDis) / player_region_size / region_manager.map_width();
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
			// double debug = config.get_weapon_parameter(WEAPON::WEAPON3).AICost;
			// DEBUGOUTPUT("Weapon3AIcost: ", debug);
			if (biuldWeapon >= config.get_weapon_parameter(WEAPON::WEAPON3).AICost && arm_level[WEAPON::WEAPON3 + 1] > 0) {
				weapons[WEAPON::WEAPON3]++;
				this->gold -= config.get_weapon_parameter(WEAPON::WEAPON3).AICost;
			}
		}
		int maxLevelCount = 0;
		for (auto level : arm_level) {
			if (level == max_level) {
				maxLevelCount++;
			}
		}

		if (maxLevelCount == weapons.size()) {
			return;
		}
		int buildLevel = (int)this->gold * uplevel_cost_weight;
		int armycost = INT_MAX;
		if (arm_level[ARMY] < max_level + 1)
			/* armycost = weaponCost["Army"].template get<std::vector<int>>()[arm_level[ARMY] - 1]; */
			armycost = army_uplevel_cost[arm_level[ARMY] - 1];
		if (buildLevel >= armycost && arm_level[ARMY] < max_level) {
			arm_level[ARMY]++;
			this->gold -= armycost;
		}
		int weapon1cost = INT_MAX;
		int weapon2cost = INT_MAX;
		int weapon3cost = INT_MAX;
		if (arm_level[WEAPON::WEAPON1 + 1] < max_level)
			/* weapon1cost = weaponCost["0"].template get<std::vector<int>>()[arm_level[WEAPON::WEAPON1 + 1]]; */
			weapon1cost = weapon_uplevel_cost[WEAPON::WEAPON1][arm_level[WEAPON::WEAPON1 + 1]];
		if (arm_level[WEAPON::WEAPON2 + 1] < max_level)
			/* weapon2cost = weaponCost["1"].template get<std::vector<int>>()[arm_level[WEAPON::WEAPON2 + 1]]; */
			weapon2cost = weapon_uplevel_cost[WEAPON::WEAPON2][arm_level[WEAPON::WEAPON2 + 1]];
		if (arm_level[WEAPON::WEAPON3 + 1] < max_level)
			/* weapon3cost = weaponCost["2"].template get<std::vector<int>>()[arm_level[WEAPON::WEAPON3 + 1]]; */
			weapon3cost = weapon_uplevel_cost[WEAPON::WEAPON3][arm_level[WEAPON::WEAPON3 + 1]];
		if (dist <= DISTANCE::SHORT) {
			if (buildLevel >= weapon1cost && arm_level[WEAPON::WEAPON1 + 1] < max_level) {
				arm_level[WEAPON::WEAPON1 + 1]++;
				this->gold -= weapon1cost;
			}
		} else if (dist <= DISTANCE::MID) {
			if (buildLevel >= weapon2cost && arm_level[WEAPON::WEAPON2 + 1] < max_level) {
				arm_level[WEAPON::WEAPON2 + 1]++;
				this->gold -= weapon2cost;
			}
		} else {
			if (buildLevel >= weapon3cost && arm_level[WEAPON::WEAPON3 + 1] < max_level) {
				arm_level[WEAPON::WEAPON3 + 1]++;
				this->gold -= weapon3cost;
			}
		}
	}

	bool isBorder(int x, int y) {
		int surroundSize = 1;
		if (region_manager.region(x, y).get_owner() == id) return false;
		for (int i = -surroundSize; i <= surroundSize; i++) {
			for (int j = -surroundSize; j <= surroundSize; j++) {
				if (x + i < 0 || x + i >= region_manager.map_width() || y + j < 0 || y + j >= region_manager.map_height()) continue;
				if (region_manager.region(x + i, y + j).get_owner() == id && !(i==0 && j==0)) {
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

	void move_army() {
		std::vector<std::pair<std::tuple<int, int>, int>> assigned_army;
		std::vector<std::pair<std::tuple<int, int>, int>> originForce;
		for (auto region : distance) {
			assigned_army.push_back(std::make_pair(region.first, 0));	
			Army& tmp = region_manager.region(std::get<0>(region.first), std::get<1>(region.first)).get_army();
			originForce.push_back(std::make_pair(region.first, tmp.get_force()));
		}
		std::vector<double> weights(cur_AI_region_size);
		int sumArmy = 0;
		int sumDistance = 0;
		for (auto AIRegion : AI_regions) {
			auto [x, y] = AIRegion;
			Region& region = region_manager.region(x, y);
			Army& regionArmy = region.get_army();
			sumArmy += regionArmy.get_force();
		}
		for (auto item : this->distance) {
			sumDistance += item.second;
		}
		for (int i = 0; i < cur_AI_region_size; i++) {
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
		for (int i = 0; i < cur_AI_region_size; i++){
			assigned_army[i].second = std::floor(sumArmy * weights[i]);
			target += assigned_army[i].second;
		}

		int remain = sumArmy - target; // ??
		remain = remain >= assigned_army.size() ? assigned_army.size() - 1 : remain;
		for (int i = 0; i < remain; i++) {
			assigned_army[i].second++;
		}
		std::vector<std::pair<std::tuple<int, int>, int>> diff;

		for (int i = 0; i < cur_AI_region_size; i++) {
			diff.push_back(std::make_pair(assigned_army[i].first, assigned_army[i].second - originForce[i].second));
		}

		std::queue<int> surplus, deficit;
		std::vector<Transaction> transactions;

		for (int i = 0; i < cur_AI_region_size; i++) {
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

		double max_time = -1;

		for (auto transaction : transactions) {
			auto [from, to, force] = transaction;
			Point start = Point(std::get<0>(from), std::get<1>(from));
			Point end = Point(std::get<0>(to), std::get<1>(to));
			int armyLevel = this->arm_level[0];
			DEBUGOUTPUT("moveArmy() calls move_Army");
			DEBUGOUTPUT("from (", start.x, ",", start.y, ")", "to", "(", end.x, ",", end.y, ")");
			try{
				max_time = std::max(max_time, region_manager.move_army(end, start, force, armyLevel));
			} catch (std::exception& e) {
				DEBUGOUTPUT("moveArmy() failed");
			}
		}
		if (max_time == -1) {
			return;
		}

		this->can_move = false;

		times.push_back(Timed_trigger(max_time, this->can_move));
		times.push_back(Timed_trigger(max_time, this->can_defend));

		/* std::thread t([this, maxTime](){ */
		/* 		this->sleep(std::ceil(maxTime)); */
		/* 		}); */
		/* t.detach(); */
		DEBUGOUTPUT("maxTime: ", max_time);
	}

	void army_attack(Point start, int amount, Point end) {
		for (auto item : is_attacked) {
			if (std::get<0>(item) == end.x && std::get<1>(item) == end.y) {
				return;
			}
		}
		this->can_move = false;
		this->can_defend = true;
		DEBUGOUTPUT("armyAttack() called");
		DEBUGOUTPUT("from (", start.x, ",", start.y, ")", "to", "(", end.x, ",", end.y, ")");
		Army& army = region_manager.region(start.x, start.y).get_army();
		int cur_force = army.get_force() * expand_army_max_weight;
		if (army.get_force() * expand_army_max_weight >= amount) {
			DEBUGOUTPUT("armyAttack only one region attack");
			double time = region_manager.move_army(start, end, cur_force, this->arm_level[0]);
			this->is_attacked.emplace_back(std::make_tuple(end.x, end.y));
			this->can_move = true;
			DEBUGOUTPUT("armyAttack only one region attack need time", time);
			return;
		}
		std::vector<Point> regionlist;
		int border_size = 1; //边界范围
		for (int i = -border_size; i <= border_size ;i++) {
			for (int j = -border_size; j <= border_size; j++) {
				if (out_of_range(start.x + i, start.y + j)) continue;
				if (region_manager.region(start.x + i, start.y + j).get_owner() != id) continue;
				Army& tmp = region_manager.region(start.x + i, start.y + j).get_army();
				cur_force +=	tmp.get_force() * expand_army_max_weight;
				regionlist.push_back(Point(start.x + i, start.y + j));
				double maxTime = 0;
				int armyLevel = this->arm_level[0];
				if (cur_force >= amount) {
					for (auto region : regionlist) {
						DEBUGOUTPUT("armyAttack() calls move_army()");
						Army& t = region_manager.region(region.x, region.y).get_army();
						maxTime = std::max(maxTime, region_manager.move_army(region, start, t.get_force() * expand_army_max_weight, armyLevel));
						DEBUGOUTPUT("move_army() finished");
					}
					this->is_attacked.emplace_back(std::make_tuple(end.x, end.y));
					DEBUGOUTPUT("ArmyAttack time: ", maxTime);
					this->can_move = false;
					/* std::this_thread::sleep_for(std::chrono::milliseconds((int)(maxTime * 1100))); */
					/* DEBUGOUTPUT("ArmyAttack finished: "); */
					/* this->canMove = true; */
					times.push_back(Timed_trigger(maxTime, this->can_move,[this, start, end, cur_force, armyLevel](){
							this->region_manager.move_army(start, end, cur_force, armyLevel);
							}));
					return;
				}
				
			}
		}
		this->can_move = true;
	}


	void expand() {
		// TODO
		/* if (!can_move) { */
		/* 	can_defend = false; */
		/* 	return; */
		/* } */
		/* can_defend = true; */
		std::vector<std::pair<std::tuple<int, int>, int>> border_distance;
		for (auto border_region : border) {
			int distance = 0;
			for (auto AI_region : AI_regions) {
				auto [x1, y1] = border_region;
				auto [x2, y2] = AI_region;
				distance += (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
			}
			border_distance.push_back(std::make_pair(border_region, distance));
		}
		std::sort(border_distance.begin(), border_distance.end(), [](const std::pair<std::tuple<int, int>, int>& a, const std::pair<std::tuple<int, int>, int>& b) {
				return a.second < b.second;
				});


		if (cur_AI_region_size < player_region_size && average_force < player_average_force) {
			// TODO
			for (int i = border_distance.size() - 1; i >= border_distance.size() / 2; i--) {
				auto [x, y] = border_distance[i].first;
				Army& borderArmy = region_manager.region(x, y).get_army();
				int borderArmyForce = borderArmy.get_force(); 
				if (borderArmyForce < average_force - 20) {
					Point maxForce(0, 0);
					int maxForceValue = 0;
					for (int j = -1; j <= 1; j++) {
						for (int k = -1; k <= 1; k++) {
							if (x + j < 0 || x + j >= region_manager.map_width() || y + k < 0 || y + k >= region_manager.map_height()) continue;
							if (region_manager.region(x + j, y + k).get_owner() == id) {
								Army& tmp = region_manager.region(x + j, y + k).get_army();
								if (tmp.get_force() >= maxForceValue) {
									maxForceValue = tmp.get_force();
									maxForce = Point(x + j, y + k);
								}
							}
						}
					}	
					bool flag = false;
					for (auto item : is_attacked) {
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

					this->can_move = false;
					this->army_attack(maxForce, borderArmyForce + 1, Point(x, y));
					/* std::thread t([this, maxForce, borderArmyForce, x, y](){ */
					/* 		this->armyAttack(maxForce, borderArmyForce + 1, Point(x, y)); */
					/* 		}); */
					/* t.detach(); */
					/* break; */
				}
			}
		} else {
			// TODO
			for (int i = 0; i <= border_distance.size() / 2; i++) {
				auto [x, y] = border_distance[i].first;
				Army& borderArmy = region_manager.region(x, y).get_army();
				int borderArmyForce = borderArmy.get_force(); 
				if (borderArmyForce < average_force - 20) {
					Point maxForce(0, 0);
					int maxForceValue = 0;
					for (int j = -1; j <= 1; j++) {
						for (int k = -1; k <= 1; k++) {
							int w = region_manager.map_width();
							int h = region_manager.map_height();
							if (x + j < 0 || x + j >= region_manager.map_width() || y + k < 0 || y + k >= region_manager.map_height()) continue;
							if (region_manager.region(x + j, y + k).get_owner() == id) {
								Army& tmp = region_manager.region(x + j, y + k).get_army();
								if (tmp.get_force() >= maxForceValue) {
									maxForceValue = tmp.get_force();
									maxForce = Point(x + j, y + k);
								}
							}
						}
					}
					bool flag = false;
					for (auto item : is_attacked) {
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
					this->can_move = false;
					this->army_attack(maxForce, borderArmyForce + 1, Point(x, y));
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
			groups[i].set_parameter(id);
		}
	}

	bool get_status() {
		return no_alive;
	}
};

