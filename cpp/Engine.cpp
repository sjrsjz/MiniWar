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
			int id = op.getSize();
			ai.setParameter(id); //1,2,3
			break;
		case Operator::setPowerStation:
			Point p = op.getCur();
			/* regionManager.get_player().set */
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
