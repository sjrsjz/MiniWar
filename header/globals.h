#pragma once
#include "passes/FragmentBuffer.h"
#include "passes/FlameRenderPass.h"
#include "Timer.h"
#include "../header/utils/Operation.h"
#include <queue>
extern Timer g_main_timer;
extern std::queue<Operation> g_main_operation;
