#ifndef __glsl_direct_tex_pass_frag__
#define __glsl_direct_tex_pass_frag__
static const char* direct_tex_pass_frag = R"(
#version 430 core

in vec2 texCoord; // [-1, 1]

layout(binding = 0) uniform sampler2D g_pass;


layout (location = 0) out vec4 fragColor;
void main(){
	vec2 uv = texCoord * 0.5 + 0.5;
	fragColor = texture(g_pass, uv);
}
)";
#endif