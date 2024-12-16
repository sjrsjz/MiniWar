const char* main_game_pass_frag = R"(
#version 430 core

in vec2 texCoord; // [-1, 1]

uniform sampler2D g_main_game_pass;


out vec4 fragColor;
void main(){
	fragColor = texture(g_main_game_pass, texCoord * 0.5 + 0.5);
}
)";