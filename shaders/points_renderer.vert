#ifndef __glsl_point_renderer_vert__
#define __glsl_point_renderer_vert__
static const char* point_renderer_vert = R"(
#version 430 core
uniform mat4 MVP;
uniform mat4 g_trans_mat;
uniform mat4 g_model_trans_mat_inv;
layout(location = 0) in vec3 vPos;
layout(location = 1) in vec4 vColor;
out vec4 color;
out vec3 world_pos;
void main()
{
	vec4 wp = inverse(g_model_trans_mat_inv * g_trans_mat) * vec4(vPos, 1.0);
	gl_Position = MVP  * wp;
	color = vColor;
	world_pos = wp.xyz;
	gl_PointSize = 50.0 / length(world_pos);
}
)";
#endif