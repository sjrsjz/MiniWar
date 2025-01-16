#ifndef __glsl_normal_gl_vert__
#define __glsl_normal_gl_vert__
static const char* normal_gl_vert = R"(
#version 430 core
uniform mat4 MVP;
uniform mat4 g_trans_mat;
uniform mat4 g_model_trans_mat_inv;
layout(location = 0) in vec3 vPos;
layout(location = 1) in vec4 vColor;
layout(location = 2) in vec2 vUV;
layout(location = 3) in vec3 vNormal;
layout(location = 4) in mat4 vModelMat;
out vec4 color;
out vec3 world_pos;
out vec3 normal;
out vec2 uv;
void main()
{
	vec4 wp = inverse(g_model_trans_mat_inv * g_trans_mat) * vModelMat *vec4(vPos, 1.0);
	gl_Position = MVP  * wp;
	color = vColor;
	world_pos = wp.xyz;
	normal = vNormal;
	uv = vUV;
}
)";
#endif