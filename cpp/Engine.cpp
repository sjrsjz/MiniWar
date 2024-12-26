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

AI ai;
AI ai2;
bool isPause = false;
bool aiState = true;
bool aiState2 = true;

void release_game() {
	RegionManager::getInstance().~RegionManager();
	ai.~AI();
	ai2.~AI();
}

static bool s_exit_game = false;
static std::thread s_main_loop;
void initial_game(int width, int height) {
	isPause = false;
	aiState = true;
	s_exit_game = false;
	RegionManager::getInstance().set(width, height);
	RegionManager::getInstance().get_player().create();
	ai.create();
	ai2.create(2);
}

static std::vector<std::string> s_error_messages;
static std::mutex s_error_mutex;

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

void read_input() {
	if (g_main_operation.empty()) return;
	Operation& op = g_main_operation.front();
	g_main_operation.pop();
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
			RegionManager::getInstance().get_player().build(op);
			break;
		case Operator::SetRefinery:
			RegionManager::getInstance().get_player().build(op);
			break;
		case Operator::SetSteelFactory:
			RegionManager::getInstance().get_player().build(op);
			break;
		case Operator::SetCivilFactory:
			RegionManager::getInstance().get_player().build(op);
			break;
		case Operator::SetMilitaryFactory:
			RegionManager::getInstance().get_player().build(op);
			break;
		case Operator::RemoveBuilding:
			RegionManager::getInstance().get_player().remove_building(op);
			break;
		case Operator::SetResearch:
			RegionManager::getInstance().get_player().set_research(op);
			break;
		case Operator::BuildingLevel:
			RegionManager::getInstance().get_player().upgrade_building(op);
			break;
		case Operator::PowerStationUpLevel:
			RegionManager::getInstance().get_player().research(op);
			break;
		case Operator::RefineryUpLevel:
			RegionManager::getInstance().get_player().research(op);
			break;
		case Operator::SteelFactoryUpLevel:
			RegionManager::getInstance().get_player().research(op);
			break;
		case Operator::CivilFactoryUpLevel:
			RegionManager::getInstance().get_player().research(op);
			break;
		case Operator::MilitaryFactoryUpLevel:
			RegionManager::getInstance().get_player().research(op);
			break;
		case Operator::ArmyUpLevel:
			RegionManager::getInstance().get_player().research(op);
			break;
		case Operator::Weapon0UpLevel:
			RegionManager::getInstance().get_player().research(op);
			break;
		case Operator::Weapon1UpLevel:
			RegionManager::getInstance().get_player().research(op);
			break;
		case Operator::Weapon2UpLevel:
			RegionManager::getInstance().get_player().research(op);
			break;
		case Operator::ArmyMove:
			RegionManager::getInstance().get_player().move_army(op, size);
			break;
		case Operator::Weapon0Attack:
			RegionManager::getInstance().get_player().attack(op);
			break;
		case Operator::Weapon1Attack:
			RegionManager::getInstance().get_player().attack(op);
			break;
		case Operator::Weapon2Attack:
			RegionManager::getInstance().get_player().attack(op);
			break;
		case Operator::RangeAttack:
			RegionManager::getInstance().get_player().rangeAttack(op);
			break;
		case Operator::ProductArmy:
			RegionManager::getInstance().get_player().product(op);
			break;
		case Operator::ProductWeapon0:
			RegionManager::getInstance().get_player().product(op);
			break;
		case Operator::ProductWeapon1:
			RegionManager::getInstance().get_player().product(op);
			break;
		case Operator::ProductWeapon2:
			RegionManager::getInstance().get_player().product(op);
			break;
		default:
			break;
	}
}

void push_input(const Operation& op) {
	g_main_operation.push(op);
}

void update() {
	RegionManager::getInstance().update(GlobalTimer::getInstance());
	ai.update(isPause, aiState);
	ai2.update(isPause, aiState2);
}

void main_loop() {

	GlobalTimer::getInstance().reset();
	while (!s_exit_game) {
		//DEBUG::DebugOutput("New Loop\n");
		GlobalTimer::getInstance().update();

		try {
			read_input();
		}
		catch (std::exception& e) {
			push_error_message(e.what());
		}

		update();
		//DEBUG::DebugOutput("End Loop\n");
		if (!aiState) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
<<<<<<< HEAD
	release_game();
=======
	DEBUG::DebugOutput("Loop Exited!");
>>>>>>> 596ebe55c0aece324369b2520e5d89441117e725
}

void exit_curr_game() {
	s_exit_game = true;
}

void run_game(int width, int height) {
	initial_game(width, height);

	// �����߳�
	s_main_loop = std::thread(main_loop);
	s_main_loop.detach();
	
}
void wait_for_exit() {
	//s_main_loop.join();
}
