#ifndef __glsl_gaussian_blur_frag__
#define __glsl_gaussian_blur_frag__
static const char* gaussian_blur_frag= R"(
#version 430 core

in vec2 texCoord; // [-1, 1]

layout(binding = 0) uniform sampler2D g_main_game_pass;
layout(binding = 1) uniform sampler2D g_last_blur_pass;

uniform bool g_from_origin;
uniform int g_blur_radius;
uniform float g_step;
uniform float g_w_div_h;
uniform bool g_vertical;
uniform bool g_gaussian;

layout (location = 0) out vec4 fragColor;

vec4 sampleData(sampler2D tex,vec2 uv){
	uv = 0.5 * uv + 0.5;
	return texture(tex,uv);
}

vec4 sampleLod(sampler2D tex, vec2 uv, float lod, float scale){
	vec4 color = vec4(0);
	float weight = 0;
	for(int i = -1; i <= 1; i++){
		for(int j = -1; j <= 1; j++){
			float w = exp(-float(i * i + j * j) );
			color += w * sampleData(tex, uv + vec2(i, j * g_w_div_h) * lod);
			weight += w;
		}
	}
	return color / weight * scale;
}

vec4 sampleData_mipmap(sampler2D tex,vec2 uv){
	uv = 0.5 * uv + 0.5;
	return texture(tex, uv, 5);
}

vec4 sampleLod_mipmap(sampler2D tex, vec2 uv, float lod, float scale){
	vec4 color = vec4(0);
	float weight = 0;
	for(int i = -1; i <= 1; i++){
		for(int j = -1; j <= 1; j++){
			float w = exp(-float(i * i + j * j) );
			color += w * sampleData_mipmap(tex, uv + vec2(i, j * g_w_div_h) * lod);
			weight += w;
		}
	}
	return color / weight * scale;
}

void main(){
	
	vec2 uv = texCoord;
	vec4 color;

	if(g_gaussian){
		if(g_vertical){
			float w = 0;
			for(int i = -g_blur_radius; i <= g_blur_radius; i++){
;				float weight = exp(- 0.05 * float(i * i));
				color += weight * sampleData(g_last_blur_pass, uv + vec2(0, i * g_w_div_h * g_step));
				w += weight;
			}
			color /= w;
		}else{
			float w = 0;
			for(int i = -g_blur_radius; i <= g_blur_radius; i++){
				float weight = exp(- 0.05 * float(i * i));
				color += weight * sampleData(g_last_blur_pass, uv + vec2(i * g_step, 0));
				w += weight;
			}
			color /= w;

		}
	}else{
	
		if(g_from_origin){
			color += sampleLod_mipmap(g_main_game_pass, uv, 0.025 * g_step, 0.25);
			color += sampleLod_mipmap(g_main_game_pass, uv, 0.005 * g_step, 0.75);
		}else{
			color += sampleLod(g_last_blur_pass, uv, 0.01 * g_step, 0.25);
			color += sampleLod(g_last_blur_pass, uv, 0.005 * g_step, 0.75);
		}
	}	
	fragColor = color;
}
)";
#endif