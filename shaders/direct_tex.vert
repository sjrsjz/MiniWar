#ifndef __glsl_direct_tex_pass_vert__
#define __glsl_direct_tex_pass_vert__
static const char* direct_tex_pass_vert = R"(
#version 430 core
uniform mat4 MVP;
in vec3 vPos;
in vec2 vUV;
out vec2 texCoord;
void main()
{
	gl_Position = MVP * vec4(vPos, 1.0);
	// 假设三角形顶点都是正负1，且只有x，y
	texCoord = vec2(vPos.x, vPos.y);
}
)";
#endif