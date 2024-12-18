
#include "../include/GL/glew.h"
#include "../include/GLFW/glfw3.h"
#include "../include/linmath.h"
#define _gl_h

#include "../header/output.h"
#include "../header/shader.h"
#include "../header/mash.h"
#include "../header/Camera.h"
#include "../header/Timer.h"

#include "../header/debug.h"
#include "../header/passes/SSBO.h"
#include "../header/globals.h"

#include "../header/utils/FloatBuffer.h"

mash s_mash;
mash triangle_mash;
SmoothCamera camera;
SmoothCamera scale_map_camera; // 缩放摄像机
Timer timer(0);

GLuint vertex_shader, fragment_shader, s_main_game_pass_program;
GLuint normal_gl_vertex_shader, normal_gl_fragment_shader, s_normal_gl_program;

GLuint map_renderer_vertex_shader, map_renderer_fragment_shader, s_map_renderer_program;
mash map_mash;
SSBO map_info_ssbo;

GLFWwindow* glfw_win;
bool keys[512];
vec3 rot;


void glfwErrorCallBack(int error, const char* str);
void glfwKeyCallBack(GLFWwindow* window, int key, int scanmode, int action, int mods);
void glfwMouseCallback(GLFWwindow* window, double xpos, double ypos);
void glfwWindowSizeCallback(GLFWwindow* window, int width, int height);
void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset);


void render_main_game_pass() {
	g_main_game_pass_fbo.bind_frameBuffer();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	int W, H;
	float ratio;
	mat4x4 m, p, mvp;

	glfwGetFramebufferSize(glfw_win, &W, &H);

	ratio =  W / (float)H;
	int mvp_location = glGetUniformLocation(s_map_renderer_program, "MVP");

	mat4x4_identity(m);
	mat4x4_ortho(p, -1, 1, -1.f, 1.f, 1.f, -1.f);
	mat4x4_mul(mvp, p, m);

	glBindBuffer(GL_ARRAY_BUFFER, map_mash.vertex_buffer);
	glUseProgram(s_map_renderer_program);
	glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);

	
	map_info_ssbo.bind(0);

	// g_fov
	glUniform1f(glGetUniformLocation(s_map_renderer_program, "g_fov"), -2.0);
	//g_frame_width, g_frame_height
	glUniform1f(glGetUniformLocation(s_map_renderer_program, "g_frame_width"), W);
	glUniform1f(glGetUniformLocation(s_map_renderer_program, "g_frame_height"), H);


	vec3 plane_u = { 0.25* cos(1), 0, 0.25 * sin(1) }, plane_v = { 0.25 * sin(1),0,-0.25 * cos(1) }, plane_pos = { 0,-2,0 };
	

	Camera model_camera;
	model_camera.setPos(0, 2, 0);
	model_camera.setRot(0, -1, 0);
	mat4x4 model_mat;
	model_camera.getMat4(model_mat);
	mat4x4_scale(model_mat, model_mat, 0.25);
	glUniformMatrix4fv(glGetUniformLocation(s_map_renderer_program, "g_model_trans_mat"), 1, GL_FALSE, (const GLfloat*)model_mat);
	model_camera.setPos(0, 2, 0);
	model_camera.setRot(0, 1, 0);
	model_camera.getMat4(model_mat);
	mat4x4_scale(model_mat, model_mat, 0.25);
	glUniformMatrix4fv(glGetUniformLocation(s_map_renderer_program, "g_model_trans_mat_inv"), 1, GL_FALSE, (const GLfloat*)model_mat);


	glUniform1f(glGetUniformLocation(s_map_renderer_program, "g_time"), (float)timer.getTime());

	glUniform2i(glGetUniformLocation(s_map_renderer_program, "g_map_size"), 64, 64);


	mat4x4 g_trans_mat;
	camera.getCamera().getMat4(g_trans_mat);
	mat4x4 g_scale_mat;
	scale_map_camera.getCamera().getMat4(g_scale_mat);
	mat4x4_mul(g_trans_mat, g_trans_mat, g_scale_mat);
	int g_trans_mat_location = glGetUniformLocation(s_map_renderer_program, "g_trans_mat");
	glUniformMatrix4fv(g_trans_mat_location, 1, GL_FALSE, (const GLfloat*)g_trans_mat);

	glDrawArrays(GL_QUADS, 0, map_mash.vertexs.size());
	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	map_info_ssbo.unbind_ssbo();

	g_main_game_pass_fbo.unbind_frameBuffer();
}

void prepare_render() {
	scale_map_camera.update(timer.getTime());
	scale_map_camera.clampZ(-4, 0,timer.getTime());

	camera.update(timer.getTime());
	camera.clampX(-5, 5, timer.getTime());
	camera.clampZ(-6, 4, timer.getTime());
}

void render() {

	int W, H;
	float ratio;
	mat4x4 m, p, mvp;

	glfwGetFramebufferSize(glfw_win,&W,&H);
	glViewport(0, 0, W, H);

	render_main_game_pass();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	

	ratio = 1;// W / (float)H;

	int mvp_location = glGetUniformLocation(s_main_game_pass_program, "MVP");
	//int fbo_location = glGetUniformLocation(s_main_game_pass_program, "g_main_game_pass");

	mat4x4_identity(m);
	mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
	mat4x4_mul(mvp, p, m);

	glBindBuffer(GL_ARRAY_BUFFER,s_mash.vertex_buffer);
	g_main_game_pass_fbo.bind_texture();
	//glUniform1i(fbo_location, g_main_game_pass_fbo.get_texture());
	glUseProgram(s_main_game_pass_program);
	glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
	glDrawArrays(GL_QUADS, 0, s_mash.vertexs.size());
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glfwSwapBuffers(glfw_win);

}

void KeyProcess() {

	float dx = 0, dz = 0;

	if (keys[GLFW_KEY_W]) {
		dz = 250;
	}
	if (keys[GLFW_KEY_S]) {
		dz = -250;
	}
	if (keys[GLFW_KEY_A]) {
		dx = 250;
	}
	if (keys[GLFW_KEY_D]) {
		dx = -250;
	}
	if (keys[GLFW_KEY_SPACE]) {

	}
	if (dx != 0 || dz != 0) {
		dx *= timer.dt;
		dz *= timer.dt;
		camera.move(dx, 0, dz, timer.getTime());

	}
	if (keys[GLFW_KEY_R]) {
		// 重置摄像机
		camera.move_to(0, 0, -2, timer.getTime());
	}
}

#include "../shaders/main_game_pass.vert"
#include "../shaders/main_game_pass.frag"
#include "../shaders/normal_gl.vert"
#include "../shaders/normal_gl.frag"
#include "../shaders/map_renderer.vert"
#include "../shaders/map_renderer.frag"

bool compileShaders() {
	DEBUG::DebugOutput("Compiling Shaders");
	s_main_game_pass_program = CompileShader(main_game_pass_vert, main_game_pass_frag, nullptr, &vertex_shader, &fragment_shader, nullptr);
	if (s_main_game_pass_program == -1) return false;
	s_normal_gl_program = CompileShader(normal_gl_vert, normal_gl_frag, nullptr, &normal_gl_vertex_shader, &normal_gl_fragment_shader, nullptr);
	if (s_normal_gl_program == -1) return false;
	s_map_renderer_program = CompileShader(map_renderer_vert, map_renderer_frag, nullptr, &map_renderer_vertex_shader, &map_renderer_fragment_shader, nullptr);
	if (s_map_renderer_program == -1) return false;
	DEBUG::DebugOutput("Shaders Compiled");
}

float randfloat() {
	return (float)rand() / RAND_MAX;
}

void init() {
	scale_map_camera.setMoveDuration(0.25);
	camera.setMoveDuration(0.5);
	camera.rotate(0, 0, -0.75, timer.getTime());
	camera.move(0, 0, -2, timer.getTime());

	DEBUG::DebugOutput("Building meshes..");
	triangle_mash.enable_color = true;
	triangle_mash.RGBA(1, 0, 0, 1);
	triangle_mash.append(-0.6f, -0.4f, 0.0f);
	triangle_mash.RGBA(0, 1, 0, 1);
	triangle_mash.append(0.6f, -0.4f, 0.0f);
	triangle_mash.RGBA(0, 0, 1, 1);
	triangle_mash.append(0.0f, 0.6f, 0.0f);
	triangle_mash.build(s_normal_gl_program, "vPos", "vColor", nullptr, nullptr);

	s_mash.enable_uv = true;
	s_mash.UV(0, 0);
	s_mash.append(1, -1, 0);
	s_mash.UV(1, 0);
	s_mash.append(1, 1, 0);
	s_mash.UV(0, 1);
	s_mash.append(-1, 1, 0);
	s_mash.UV(0, 0);
	s_mash.append(-1, -1, 0);
	s_mash.build(s_main_game_pass_program, "vPos", nullptr, "vUV", nullptr);

	map_mash.append(1, -1, 0);
	map_mash.append(1, 1, 0);
	map_mash.append(-1, 1, 0);
	map_mash.append(-1, -1, 0);
	map_mash.build(s_map_renderer_program, "vPos", nullptr, nullptr, nullptr);
	DEBUG::DebugOutput("Meshes built");

	DEBUG::DebugOutput("Creating SSBO..");

	DATA::FloatBuffer map_info;

	for (int i{}; i < 64; i++) {
		for (int j{}; j < 64; j++) {
			map_info << randfloat() << randfloat() << (float)(rand() % 2) << 0.0;
		}
	}

	map_info_ssbo = SSBO(map_info.size() * sizeof(float), GL_STATIC_DRAW);
	map_info_ssbo.set_binding_point_index(0);
	map_info_ssbo.create_ssbo();
	map_info_ssbo.update_data(map_info.buffer().get(), map_info.size() * sizeof(float));




	DEBUG::DebugOutput("SSBO Created");
	timer.setTime(glfwGetTime());

}

int main() {
	if (!glfwInit()) {
		println("Failed to initialize glfw"); return 0;
	}
	glfwSetErrorCallback((GLFWerrorfun)glfwErrorCallBack);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfw_win=glfwCreateWindow(800,500,"MiniWar",NULL,NULL);

	glfwSetKeyCallback(glfw_win, (GLFWkeyfun)glfwKeyCallBack);
	glfwSetCursorPosCallback(glfw_win, (GLFWcursorposfun)glfwMouseCallback);
	glfwSetWindowSizeCallback(glfw_win, (GLFWwindowsizefun)glfwWindowSizeCallback);
	glfwSetScrollCallback(glfw_win, (GLFWscrollfun)glfwScrollCallback);

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


#ifdef _DEBUG
	glDebugMessageCallback((GLDEBUGPROC)debugProc, 0);
	glEnable(GL_DEBUG_CALLBACK_FUNCTION);
#endif // DEBUG

	DEBUG::DebugOutput("OpenGL Version", (std::string)(char*)glGetString(GL_VERSION));
	DEBUG::DebugOutput("GLSL Version", (std::string)(char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

	g_main_game_pass_fbo = FragmentBuffer(800, 500);


	if (!compileShaders()) goto destroy;

	init();

	while (!glfwWindowShouldClose(glfw_win))
	{
		timer.setTime(glfwGetTime());
		prepare_render();
		KeyProcess();
		render();
		glfwPollEvents();
	}
destroy:

	g_main_game_pass_fbo.release();

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

// 滚轮事件
void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	scale_map_camera.move(0, 0, yoffset, timer.getTime());
}

void glfwWindowSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	g_main_game_pass_fbo.resize(width, height);
}