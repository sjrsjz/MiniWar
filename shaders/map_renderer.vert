#ifndef __glsl_map_renderer_vert__
#define __glsl_map_renderer_vert__
static const char* map_renderer_vert = R"(
#version 430 core
uniform mat4 MVP;
layout(location = 0) in vec3 vPos;
out vec2 texCoord;
void main()
{
	gl_Position = MVP * vec4(vPos, 1.0);
	// 假设三角形顶点都是正负1，且只有x，y
	texCoord = vec2(vPos.x, vPos.y);
}
)";
#endif