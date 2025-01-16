#ifndef __glsl_normal_gl_frag__
#define __glsl_normal_gl_frag__
static const char* normal_gl_frag = R"(
#version 430 core

layout(binding = 0) uniform sampler2D tex;
in vec4 color;
in vec3 world_pos;
in vec3 normal;
in vec2 uv;

out vec4 fragColor;
void main(){
	fragColor = color * max(0.2, dot(normalize(normal),normalize(vec3(1,1,1)))) * texture(tex, uv);
	float depth = length(world_pos);
	gl_FragDepth = depth/(1+depth);
}
)";
#endif