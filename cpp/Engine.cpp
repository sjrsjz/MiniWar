#include "../header/Engine.h"
#include "../header/Logic/AI.h"
#include "../header/globals.h"
#include "../header/utils/Config.h"
#include "../header/utils/Operation.h"
#include <thread>
#include <queue>

AI ai;
bool isPause = false;

static bool s_exit_game = false;
static std::thread s_main_loop;
void initial_game(int width, int height) {
	s_exit_game = false;
	RegionManager::getInstance().set(width, height);
	RegionManager::getInstance().get_player().create();
	ai.create();
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
	ai.update(isPause);
}

void main_loop() {
	GlobalTimer::getInstance().reset();
	while (!s_exit_game) {
		//DEBUG::DebugOutput("New Loop\n");
		GlobalTimer::getInstance().update();

		read_input();


		update();
		//DEBUG::DebugOutput("End Loop\n");
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}


void run_game(int width, int height) {
	initial_game(width, height);

	// Æô¶¯Ïß³Ì
	s_main_loop = std::thread(main_loop);
	s_main_loop.detach();
	
}
void wait_for_exit() {
	s_main_loop.join();
}
