const char* main_game_pass_frag = R"(
#version 430 core

in vec2 texCoord; // [-1, 1]

layout(binding = 0) uniform sampler2D g_main_game_pass;
layout(binding = 1) uniform sampler2D g_blur_pass;
uniform float g_blur = 0.1;

vec3 ACES(const vec3 x){
	const float a = 2.51;
	const float b = 0.03;
	const float c = 2.43;
	const float d = 0.59;
	const float e = 0.14;
	return clamp((x*(a*x+b))/(x*(c*x+d)+e),0,1);
}


layout (location = 0) out vec4 fragColor;
void main(){
	vec2 uv = texCoord * 0.5 + 0.5;
	vec4 color = 0.5 * mix(texture(g_main_game_pass, uv), texture(g_blur_pass, uv), g_blur);
	color.rgb = ACES(color.rgb);
	fragColor.rgb = pow(color.rgb, vec3(1/2.2));
	fragColor.a = 1;
}
)";