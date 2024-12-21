#pragma once
#include "passes/FragmentBuffer.h"
#include "passes/FlameRenderPass.h"
#include "Timer.h"
extern FragmentBuffer g_final_mix_pass_fbo;
extern FragmentBuffer g_main_game_pass_fbo;
extern FragmentBuffer g_gaussian_blur_pass_fbo;
extern FragmentBuffer g_gaussian_blur_vertical_pass_fbo;
extern FlameRenderPass g_flame_render_pass;
extern Timer g_main_timer;