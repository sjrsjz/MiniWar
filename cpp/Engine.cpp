#include "../header/Engine.h"
#include "../header/Logic/AI.h"
#include "../header/globals.h"
#include "../header/utils/Config.h"
#include "../header/utils/Operation.h"
#include <queue>

AI ai;
bool isPause = false;
RegionManager regionManager(0, 0);
Config config("./config.json");
GlobalTimer timer;

void initial_game(int width, int height) {
	regionManager.set(width, height);
}

void read_input() {
	Operation& op = g_main_operation.front();
	g_main_operation.pop();
	int id = op.getSize();
	Point p = op.getCur();
	int size = op.getSize();
	switch (op.getOp()) {
		case Operator::Quit:
			exit(0);
			break;
		case Operator::Pause:
			isPause = true;
			
			break;
		case Operator::Start:
			isPause = false;
			break;
		case Operator::MapSet:
			ai.setParameter(id); //1,2,3
			break;
		case Operator::SetPowerStation:
			regionManager.get_player().build(op);
			break;
		case Operator::SetRefinery:
			regionManager.get_player().build(op);
			break;
		case Operator::SetSteelFactory:
			regionManager.get_player().build(op);
			break;
		case Operator::SetCivilFactory:
			regionManager.get_player().build(op);
			break;
		case Operator::SetMilitaryFactory:
			regionManager.get_player().build(op);
			break;
		case Operator::RemoveBuilding:
			regionManager.get_player().remove_building(op);
			break;
		case Operator::SetResearch:
			regionManager.get_player().build_research(op);
			break;
		case Operator::BuildingLevel:
			regionManager.get_player().upgrade_building(op);
			break;
		case Operator::PowerStationUpLevel:
			regionManager.get_player().research(op);
			break;
		case Operator::RefineryUpLevel:
			regionManager.get_player().research(op);
			break;
		case Operator::SteelFactoryUpLevel:
			regionManager.get_player().research(op);
			break;
		case Operator::CivilFactoryUpLevel:
			regionManager.get_player().research(op);
			break;
		case Operator::MilitaryFactoryUpLevel:
			regionManager.get_player().research(op);
			break;
		case Operator::ArmyUpLevel:
			regionManager.get_player().research(op);
			break;
		case Operator::Weapon0UpLevel:
			regionManager.get_player().research(op);
			break;
		case Operator::Weapon1UpLevel:
			regionManager.get_player().research(op);
			break;
		case Operator::Weapon2UpLevel:
			regionManager.get_player().research(op);
			break;
		case Operator::Product:
			regionManager.get_player().product(op);
			break;
		case Operator::ArmyMove:
			regionManager.get_player().move_army(op, size);
			break;
		case Operator::Weapon0Attack:
			regionManager.get_player().attack(op);
			break;
		case Operator::Weapon1Attack:
			regionManager.get_player().attack(op);
			break;
		case Operator::Weapon2Attack:
			regionManager.get_player().attack(op);
			break;
		case Operator::RangeAttack:
			regionManager.get_player().range_attack(op);
			break;
		case Operator::ProductArmy:
			regionManager.get_player().product(op);
			break;
		case Operator::ProductWeapon0:
			regionManager.get_player().product(op);
			break;
		case Operator::ProductWeapon1:
			regionManager.get_player().product(op);
			break;
		case Operator::ProductWeapon2:
			regionManager.get_player().product(op);
			break;
		default:
			break;
	}
}

void update() {
	regionManager.update(timer);
	ai.update(isPause);
}

void main_loop() {
	while (true) {
		timer.start();

		read_input();

		timer.stop();
		update();
	}
}

void run_game(int width, int height) {
	initial_game(width, height);
	main_loop();
	//release resoure
}
