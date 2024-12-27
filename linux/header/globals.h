#ifndef GLOBALS_H
#define GLOBALS_H
#include "passes/FragmentBuffer.h"
#include "passes/FlameRenderPass.h"
#include "Timer.h"
#include "../header/utils/Operation.h"
#include <queue>
extern FragmentBuffer g_final_mix_pass_fbo;
extern FragmentBuffer g_main_game_pass_fbo;
extern FragmentBuffer g_gaussian_blur_pass_fbo;
extern FragmentBuffer g_gaussian_blur_vertical_pass_fbo;
extern FlameRenderPass g_flame_render_pass;
extern FragmentBuffer g_point_render_pass_fbo;
extern Timer g_main_timer;
extern std::queue<Operation> g_main_operation;
#endif