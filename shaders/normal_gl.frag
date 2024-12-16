const char* normal_gl_frag = R"(
#version 430 core

in vec4 color;

out vec4 fragColor;
void main(){
	fragColor = color;	
}
)";