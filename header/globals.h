#pragma once
#include "passes/FragmentBuffer.h"
#include "Timer.h"
#include "../header/utils/Operation.h"
#include <queue>
extern FragmentBuffer g_main_game_pass_fbo;

extern Timer g_main_timer;
extern std::queue<Operation> g_main_operation;
