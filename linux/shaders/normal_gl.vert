#ifndef __glsl_normal_gl_vert__
#define __glsl_normal_gl_vert__
static const char* normal_gl_vert = R"(
#version 430 core
uniform mat4 MVP;
layout(location = 0) in vec3 vPos;
layout(location = 1) in vec4 vColor;
out vec4 color;
void main()
{
	gl_Position = MVP * vec4(vPos, 1.0);
	color = vColor;
}
)";
#endif