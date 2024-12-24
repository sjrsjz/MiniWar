#pragma once

#include "../../header/utils/GlobalTimer.h"
#include "../../header/Logic/RegionManager.h"

RegionManager& initial_game(int width, int height);
void main_loop(RegionManager& regionmanager);
void read_input();
void update(GlobalTimer& timer, RegionManager& regionmanager);
void run_game(int width, int height);