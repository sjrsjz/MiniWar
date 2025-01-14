#ifndef __glsl_normal_gl_frag__
#define __glsl_normal_gl_frag__
static const char* normal_gl_frag = R"(
#version 430 core

in vec4 color;

out vec4 fragColor;
void main(){
	fragColor = color;	
}
)";
#endif