
#include "../include/GL/glew.h"
#include "../include/GLFW/glfw3.h"
#include "../include/linmath.h"
#define _gl_h

#include "../header/output.h"
#include "../header/shader.h"
#include "../header/mash.h"
#include "../header/Camera.h"
#include "../header/Timer.h"

mash mash_;
Camera camera;
Timer timer(0);

GLuint vertex_buffer, vertex_shader, fragment_shader, program;
int mvp_location;
GLFWwindow* glfw_win;
bool keys[512];
vec3 rot;

static const char* vertex_shader_text =
"#version 430 core\n"
"uniform mat4 MVP;\n"
"in vec3 vPos;\n"
"in vec4 vCol;\n"

"out vec4 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 1.0);\n"
"    color = vCol;\n"
"}\n";

static const char* fragment_shader_text =
"#version 430 core\n"
"in vec4 color;\n"
"out vec4 frag;\n"
"void main()\n"
"{\n"
"    frag = color;\n"
"}\n";



void glfwErrorCallBack(int error, const char* str);
void glfwKeyCallBack(GLFWwindow* window, int key, int scanmode, int action, int mods);
void glfwMouseCallback(GLFWwindow* window, double xpos, double ypos);


void render() {
	int W, H;
	float ratio;
	mat4x4 m, p, mvp;

	glfwGetFramebufferSize(glfw_win,&W,&H);
	glViewport(0, 0, W, H);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_FUNC);
	

	ratio = W / (float)H;

	mvp_location = glGetUniformLocation(program, "MVP");
	mat4x4_identity(m);
	//mat4x4_rotate_Z(m, m, (float)glfwGetTime());
	mat4x4_mul(m, m, camera.m);

	mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
	mat4x4_mul(mvp, p, camera.m);

	glBindBuffer(GL_ARRAY_BUFFER,mash_.vertex_buffer);
	glUseProgram(program);
	glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
	glDrawArrays(GL_QUADS, 0, mash_.vertexs.size());
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glfwSwapBuffers(glfw_win);
}

void KeyProcess() {

	if (keys[GLFW_KEY_W]) camera.move_local(0, 0, timer.dt, rot[0], rot[1], rot[2]);
	if (keys[GLFW_KEY_A]) camera.move_local(-timer.dt, 0, 0, rot[0], rot[1], rot[2]);
	if (keys[GLFW_KEY_D]) camera.move_local(timer.dt, 0, 0, rot[0], rot[1], rot[2]);
	if (keys[GLFW_KEY_S]) camera.move_local(0, 0, -timer.dt, rot[0], rot[1], rot[2]);

}
int main() {
	if (!glfwInit()) {
		println("Failed to initialize glfw"); return 0;
	}
	glfwSetErrorCallback((GLFWerrorfun)glfwErrorCallBack);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfw_win=glfwCreateWindow(800,500,"OpenGL",NULL,NULL);
	glfwSetKeyCallback(glfw_win, (GLFWkeyfun)glfwKeyCallBack);
	glfwSetCursorPosCallback(glfw_win, (GLFWcursorposfun)glfwMouseCallback);

	glfwMakeContextCurrent(glfw_win);
	glfwSwapInterval(1);
	if (!glfw_win) {
		println("Failed to create window"); return 0;
	}
	int err = glewInit();
	if (err) {
		println("Failed to initialize glew");
		println((char*)glewGetErrorString(err));
		goto destroy;
	}


#ifdef DEBUG
	glDebugMessageCallback((GLDEBUGPROC)debugProc, 0);
	glEnable(GL_DEBUG_CALLBACK_FUNCTION);
#endif // DEBUG

	program = CompileShader(vertex_shader_text, fragment_shader_text, nullptr, &vertex_shader, &fragment_shader, nullptr);
	if (program == -1) goto destroy;

	mash_.enable_color = true;
	mash_.RGBA(1, 0, 0, 1);
	mash_.append(1, 0, 0);
	mash_.RGBA(0, 1, 0, 1);
	mash_.append(1, 1, 0);
	mash_.RGBA(0, 0, 1, 1);
	mash_.append(0, 1, 0);
	mash_.RGBA(1, 0, 1, 1);
	mash_.append(0, 0, 0);
	mash_.build(program, "vPos", "vCol", nullptr, nullptr);
	timer.setTime(glfwGetTime());

	while (!glfwWindowShouldClose(glfw_win))
	{
		timer.setTime(glfwGetTime());
		KeyProcess();
		render();
		glfwPollEvents();
	}
destroy:
	glfwDestroyWindow(glfw_win);
	glfwTerminate();
	return 0;
}
void glfwErrorCallBack(int error, const char* str) {
	println(str);
}
void glfwKeyCallBack(GLFWwindow* window, int key, int scanmode, int action, int mods) {
	keys[key] = (bool)action;
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(glfw_win, GLFW_TRUE); break;
		default:
			break;

		}
		return;
	}

}
void glfwMouseCallback(GLFWwindow* window, double xpos, double ypos) {
	rot[1] = xpos/100; rot[0] = ypos/100;
}