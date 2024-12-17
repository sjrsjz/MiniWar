const char* normal_gl_vert = R"(
#version 430 core
uniform mat4 MVP;
in vec3 vPos;
in vec4 vColor;
out vec4 color;
void main()
{
	gl_Position = MVP * vec4(vPos, 1.0);
	color = vColor;
}
)";