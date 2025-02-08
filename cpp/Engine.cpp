#include "../header/Engine.h"
#include "../header/Logic/AI.h"
#include "../header/globals.h"
#include "../header/utils/Config.h"
#include "../header/utils/Operation.h"
#include <thread>
#include <queue>
#include <mutex>
#include <iostream>
#include <string>
#include <vector>

/* AI ai; */
/* AI ai2; */
bool isPause = false;
/* bool aiState = true; */
/* bool aiState2 = true; */
// 指定AI的数量
AI_groups ai(2);

void release_game() {
	ai.clear();
}

static bool s_exit_game = false;
static std::thread s_main_loop;
void initial_game(int width, int height) {
	isPause = false;
	/* aiState = true; */
	/* aiState2 = true; */
	s_exit_game = false;
	RegionManager::instance_of().init(width, height);
	RegionManager::instance_of().get_player().create();
	/* ai.create(1); */
	/* ai2.create(2); */
	/* ai.setParameter(1); */
	/* ai2.setParameter(1); */
	ai.set_parameter(1);
}

static std::vector<std::string> s_error_messages;
static std::mutex s_error_mutex;
static std::mutex s_wait_lock;

void push_error_message(const std::string& msg) {
	std::lock_guard<std::mutex> lock(s_error_mutex);
	s_error_messages.push_back(msg);
}

std::vector<std::string> get_error_messages() {
	std::lock_guard<std::mutex> lock(s_error_mutex);
	std::vector<std::string> result = s_error_messages;
	s_error_messages.clear();
	return result;
}

static std::vector<GameEffect> s_game_effects;
static std::mutex s_game_effects_mutex;

void push_game_effects(GameEffect effect) {
	std::lock_guard<std::mutex> lock(s_game_effects_mutex);
	s_game_effects.push_back(effect);
}

std::vector<GameEffect> get_game_effects() {
	std::lock_guard<std::mutex> lock(s_game_effects_mutex);
	std::vector<GameEffect> result = s_game_effects;
	s_game_effects.clear();
	return result;
}



int result_dict_counter = 0;
std::map<int, std::string> result_dict{};
std::mutex result_dict_mutex;
std::mutex g_main_operation_mutex;

void read_input(int& idx) {
	std::lock_guard<std::mutex> lock(g_main_operation_mutex);
	if (g_main_operation.empty()) return;
	Operation op = g_main_operation.front();
	g_main_operation.pop();
	idx = op.idx;
	int id = op.getSize();
	Point p = op.getCur();
	int size = op.getSize();
	switch (op.getOp()) {
		case Operator::Quit:
			s_exit_game = true;
			break;
		case Operator::Pause:
			isPause = true;			
			break;
		case Operator::Start:
			isPause = false;
			break;
		case Operator::MapSet:
			//ai.setParameter(id); //1,2,3
			break;
		case Operator::SetPowerStation:
			RegionManager::instance_of().get_player().build(op);
			break;
		case Operator::SetRefinery:
			RegionManager::instance_of().get_player().build(op);
			break;
		case Operator::SetSteelFactory:
			RegionManager::instance_of().get_player().build(op);
			break;
		case Operator::SetCivilFactory:
			RegionManager::instance_of().get_player().build(op);
			break;
		case Operator::SetMilitaryFactory:
			RegionManager::instance_of().get_player().build(op);
			break;
		case Operator::RemoveBuilding:
			RegionManager::instance_of().get_player().remove_building(op);
			break;
		case Operator::SetResearch:
			RegionManager::instance_of().get_player().set_research(op);
			break;
		case Operator::BuildingLevel:
			RegionManager::instance_of().get_player().upgrade_building(op);
			break;
		case Operator::PowerStationUpLevel:
			RegionManager::instance_of().get_player().research(op);
			break;
		case Operator::RefineryUpLevel:
			RegionManager::instance_of().get_player().research(op);
			break;
		case Operator::SteelFactoryUpLevel:
			RegionManager::instance_of().get_player().research(op);
			break;
		case Operator::CivilFactoryUpLevel:
			RegionManager::instance_of().get_player().research(op);
			break;
		case Operator::MilitaryFactoryUpLevel:
			RegionManager::instance_of().get_player().research(op);
			break;
		case Operator::ArmyUpLevel:
			RegionManager::instance_of().get_player().research(op);
			break;
		case Operator::Weapon0UpLevel:
			RegionManager::instance_of().get_player().research(op);
			break;
		case Operator::Weapon1UpLevel:
			RegionManager::instance_of().get_player().research(op);
			break;
		case Operator::Weapon2UpLevel:
			RegionManager::instance_of().get_player().research(op);
			break;
		case Operator::ArmyMove:
			RegionManager::instance_of().get_player().move_army(op, size);
			break;
		case Operator::Weapon0Attack:
			RegionManager::instance_of().get_player().attack(op);
			break;
		case Operator::Weapon1Attack:
			RegionManager::instance_of().get_player().attack(op);
			break;
		case Operator::Weapon2Attack:
			RegionManager::instance_of().get_player().attack(op);
			break;
		case Operator::RangeAttack:
			RegionManager::instance_of().get_player().range_attack(op);
			break;
		case Operator::ProductArmy:
			RegionManager::instance_of().get_player().product(op);
			break;
		case Operator::ProductWeapon0:
			RegionManager::instance_of().get_player().product(op);
			break;
		case Operator::ProductWeapon1:
			RegionManager::instance_of().get_player().product(op);
			break;
		case Operator::ProductWeapon2:
			RegionManager::instance_of().get_player().product(op);
			break;
		default:
			throw std::invalid_argument("Invalid Operator");
			break;
	}
}

int __push_input(const Operation& op) {
	std::lock_guard<std::mutex> lock(result_dict_mutex);
	std::lock_guard<std::mutex> queue_lock(g_main_operation_mutex);
	result_dict_counter++;
	Operation tmp = op;
	tmp.idx = result_dict_counter;
	g_main_operation.push(std::move(tmp));
	DEBUGOUTPUT("Pushed: ", g_main_operation.back().idx);
	return tmp.idx;
}
std::string wait_for_result(int idx) {
	std::string result = "";
	while (true) {
		result_dict_mutex.lock();
		if (result_dict.size() > 0) {
			if (result_dict.find(idx) != result_dict.end()) {
				// 移除
				result = result_dict[idx];
				result_dict.erase(idx);
				result_dict_mutex.unlock();
				DEBUGOUTPUT("Got Result: ", result, "Idx", idx);
				break;
			}
			//DEBUGOUTPUT("Waiting for Result: ", idx);
		}
		result_dict_mutex.unlock();
		// 1ms
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	return result;
}
void push_input(const Operation& op) {
	int idx = __push_input(op);
	wait_for_result(idx);
}

std::string push_input_wait_for_result(const Operation& op) {
	int idx = __push_input(op);
	return wait_for_result(idx);
}

void update() {
	RegionManager::instance_of().update(GlobalTimer::instance_of());
	RegionManager::instance_of().get_player().update(GlobalTimer::instance_of());
	/* ai.update(isPause, aiState); */
	/* ai2.update(isPause, aiState2); */
	ai.update();
}

bool g_game_stop = false;
bool g_game_over = false;

void pause_game(bool pause) {
	if (pause && !isPause) {
		isPause = true;
		GlobalTimer::instance_of().pause();
	}
	else if (!pause && isPause) {
		isPause = false;
		GlobalTimer::instance_of().resume();
	}
}

void main_loop() {
	g_game_over = false;
	g_game_stop = false;
	s_wait_lock.lock();
	GlobalTimer::instance_of().reset();
	while (!s_exit_game) {
		if ((!ai.get_status()) && RegionManager::instance_of().get_player().is_alive()) {
			//DEBUGOUTPUT("New Loop\n");
			GlobalTimer::instance_of().update();

			result_dict_mutex.lock();
			int idx = -1;
			std::string result = "";
			try {
				read_input(idx);
				result = "Success";
			}
			catch (std::exception& e) {
				push_error_message(e.what());
				result = "Error";
			}
			result_dict[idx] = result;
			if(idx != -1)
				DEBUGOUTPUT("Result: ", result, "Idx", idx);
			result_dict_mutex.unlock();

			update();
			//DEBUGOUTPUT("End Loop\n");

		}
		else {
			g_game_stop = true;
			if (!RegionManager::instance_of().get_player().is_alive()) {
				g_game_over = true;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	release_game();
	DEBUGOUTPUT("Loop Exited!");
	s_wait_lock.unlock();
}

void exit_curr_game() {
	s_exit_game = true;
}

void run_game(int width, int height) {
	initial_game(width, height);

	s_main_loop = std::thread(main_loop);
	s_main_loop.detach();
	
}
void wait_for_exit() {
	s_wait_lock.lock();
	s_wait_lock.unlock();
}
