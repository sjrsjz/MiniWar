#include "../../header/Engine.h"

RegionManager& initial_game(int width, int height) {
	RegionManager region_manager(width, height);
	return region_manager;
}

void read_input() {

}

void update(GlobalTimer& timer, RegionManager& regionmanager) {

}

void main_loop(RegionManager& regionmanager) {
	GlobalTimer& timer = GlobalTimer::getInstance();
	while (true) {
		timer.start();

		read_input();

		timer.stop();
		update(timer, regionmanager);
	}
}

void run_game(int width, int height) {
	RegionManager& regionmanager = initial_game(width, height);
	main_loop(regionmanager);
	//release resoure
}