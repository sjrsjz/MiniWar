
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

#include "../header/passes/SSBOByteArray.h"

mash s_mash;
mash triangle_mash;
Camera camera;
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
	//map_info_ssbo.bind_ssbo();
	//GLuint mapBufferIndex = glGetProgramResourceIndex(s_map_renderer_program, GL_SHADER_STORAGE_BLOCK, "MapBuffer");
	//if (mapBufferIndex != GL_INVALID_INDEX) {
	//	glShaderStorageBlockBinding(s_map_renderer_program, mapBufferIndex, map_info_ssbo.get_binding_point_index());
	//}
	//else {
	//	DEBUG::DebugOutput("mapBuffer not found in shader");
	//}
	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, map_info_ssbo.get_ssbo());

	// g_fov
	glUniform1f(glGetUniformLocation(s_map_renderer_program, "g_fov"), -2.0);
	//g_frame_width, g_frame_height
	glUniform1f(glGetUniformLocation(s_map_renderer_program, "g_frame_width"), W);
	glUniform1f(glGetUniformLocation(s_map_renderer_program, "g_frame_height"), H);

	glUniform1f(glGetUniformLocation(s_map_renderer_program, "g_time"), (float)timer.getTime());

	mat4x4 g_trans_mat;
	camera.getMat4(g_trans_mat);
	int g_trans_mat_location = glGetUniformLocation(s_map_renderer_program, "g_trans_mat");
	glUniformMatrix4fv(g_trans_mat_location, 1, GL_FALSE, (const GLfloat*)g_trans_mat);

	glDrawArrays(GL_QUADS, 0, map_mash.vertexs.size());
	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	map_info_ssbo.unbind_ssbo();

	g_main_game_pass_fbo.unbind_frameBuffer();
}


void render() {
	int W, H;
	float ratio;
	mat4x4 m, p, mvp;

	glfwGetFramebufferSize(glfw_win,&W,&H);
	glViewport(0, 0, W, H);

	render_main_game_pass();
	//return;

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

void init() {
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

	DATA::SSBOByteArray<unsigned char> map_info;
	map_info_ssbo = SSBO(16 * sizeof(float), GL_STATIC_DRAW);
	map_info_ssbo.set_binding_point_index(0);
	map_info_ssbo.create_ssbo();
	map_info << 1.0f << 1.0f << 0.0f << 0.0f;
	map_info << 0.0f << 1.0f << 0.0f << 0.0f;
	map_info << 0.0f << 0.0f << 1.0f << 0.0f;
	map_info << 0.0f << 0.0f << 0.0f << 0.0f;
	
	map_info_ssbo.update_data(map_info.ptr, map_info.size);




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

void glfwWindowSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	g_main_game_pass_fbo.resize(width, height);
}