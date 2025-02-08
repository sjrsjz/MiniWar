#pragma once

#include "../header/utils/GlobalTimer.h"
#include "../header/Logic/RegionManager.h"
#include "../header/Logic/Resource.h"
#include "../header/Logic/GameEffect.h"

void initial_game(int width, int height);
void main_loop();
void read_input();
void update();
void run_game(int width, int height);
void wait_for_exit();
void exit_curr_game();
void push_input(const Operation& op);
std::string push_input_wait_for_result(const Operation& op);
void push_error_message(const std::string& msg);
std::vector<std::string> get_error_messages();
void push_game_effects(GameEffect effect);
std::vector<GameEffect> get_game_effects();
void pause_game(bool pause);
