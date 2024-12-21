
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
#include "../header/utils/RegionSSBOBuffer.h"
#include "../header/utils/RegionData.h"
#include "../header/globals.h"

#include "../header/utils/FloatBuffer.h"
#include "../header/utils/RegionSelector.h"

#include "../include/imgui/imgui.h"
#include "../include/imgui/imgui_impl_glfw.h"
#include "../include/imgui/imgui_impl_opengl3.h"

#include "../header/utils/ImageLoader.h"

#include "../shaders/main_game_pass.vert"
#include "../shaders/main_game_pass.frag"
#include "../shaders/normal_gl.vert"
#include "../shaders/normal_gl.frag"
#include "../shaders/map_renderer.vert"
#include "../shaders/map_renderer.frag"
#include "../shaders/gaussian_blur.vert"
#include "../shaders/gaussian_blur.frag"
#include "../shaders/direct_tex.frag"
#include "../shaders/direct_tex.vert"

mash s_mash;
SmoothCamera camera;
SmoothCamera scale_map_camera; // 缩放摄像机
Timer timer(0);

GLuint vertex_shader, fragment_shader, s_main_game_pass_program;
GLuint normal_gl_vertex_shader, normal_gl_fragment_shader, s_normal_gl_program;

// 直接渲染纹理
GLuint direct_tex_vertex_shader, direct_tex_fragment_shader, s_direct_tex_program;

GLuint map_renderer_vertex_shader, map_renderer_fragment_shader, s_map_renderer_program;
mash map_mash;
RegionSSBOBuffer map_info(64, 64);

SmoothMove map_rotation;

GLuint gaussian_blur_vertex_shader, gaussian_blur_fragment_shader, s_gaussian_blur_program;


GLFWwindow* glfw_win;
bool keys[512];
vec3 s_mouse_position;

int s_current_selected_grid[2] = { -1,-1 };
bool s_is_selected = false;


namespace TEXTURE {
	GLuint s_image_radioactive;
	GLuint s_image_attack_target;
	GLuint s_image_scatter;
	GLuint s_image_lightning;
}

bool s_pause_rendering = false;

void glfwErrorCallBack(int error, const char* str);
void glfwKeyCallBack(GLFWwindow* window, int key, int scanmode, int action, int mods);
void glfwMouseCallback(GLFWwindow* window, double xpos, double ypos);
void glfwWindowSizeCallback(GLFWwindow* window, int width, int height);
void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void KeyProcess();
void KeyRelease(int key);

enum SelectedWeapon {
	NUCLEAR_MISSILE, // 核导弹
	ARMY, // 军队
	SCATTER_BOMB, // 散弹炸弹
} s_selected_weapon;

namespace UIFonts {
	ImFont* default_font;
	ImFont* menu_font;
	ImFont* large_font;
}


class SelectedGui {
public:
	int grid[2]{}; // 选中的格子
	bool is_selected = false; // 是否选中
	
	void render_gui(ImGuiIO& io) {
		if (!is_selected) return;
		ImGui::SetNextWindowBgAlpha(0.5);
		ImGui::Begin("Selected Grid", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing);
		// 移动到最低端
		ImGui::SetWindowPos(ImVec2(io.DisplaySize.x / 8 * 3, io.DisplaySize.y - 200));
		ImGui::SetWindowSize(ImVec2(io.DisplaySize.x / 4, 200));


		ImGui::Text("Selected Grid: %d, %d", grid[0], grid[1]);
		ImGui::End();
	}

}s_selected_gui;

class MenuGui {
	bool open = false;
	SmoothMove x{};
public:
	MenuGui() {
		x.setTotalDuration(0.5);
	}
	void open_gui(bool open, const Timer& timer) {
		this->open = open;
		if (open) {
			x.newEndPosition(1, timer.getTime());
		}
		else {
			x.newEndPosition(0, timer.getTime());
		}
	}
	bool is_activitied() {
		return x.getX() > 1e-3;
	}
	void render_gui(ImGuiIO& io) {
		if (!is_activitied()) {
			return;
		}
		ImGui::SetNextWindowBgAlpha(0.25 * x.getX());
		// 设置边框宽度为0
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		ImGui::Begin("Menu", nullptr,
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			//ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar
		);

		int x_pos = 0;
		ImGui::SetWindowPos(ImVec2(x_pos, 0));
		ImGui::SetWindowSize(io.DisplaySize);
		// 调整字体大小(不使用缩放而是直接使用大号字体）
		ImGui::PushFont(UIFonts::menu_font);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, x.getX()));
		const char* tilte = u8"菜单";
		ImVec2 title_size = ImGui::CalcTextSize(tilte);
		ImGui::SetCursorScreenPos(ImVec2((io.DisplaySize.x - title_size.x) / 2 + x_pos, 50));
		ImGui::Text(tilte);
		ImGui::PopStyleColor();
		ImGui::PopFont();

		// 按钮
		ImGui::PushFont(UIFonts::large_font);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25, 0.25, 0.25, 0.25 * x.getX()));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35, 0.35, 0.35, x.getX()));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15, 0.15, 0.15, x.getX()));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, x.getX()));
		ImGui::SetCursorScreenPos(ImVec2((io.DisplaySize.x - 400) / 2 + x_pos, 200));
		if (ImGui::Button(u8"继续游戏", ImVec2(400, 100))) {
			open_gui(false, timer);
		}

		ImGui::SetCursorScreenPos(ImVec2((io.DisplaySize.x - 400) / 2 + x_pos, 360));
		if (ImGui::Button(u8"退出游戏", ImVec2(400, 100))) {
			glfwSetWindowShouldClose(glfw_win, true);
		}
		ImGui::PopStyleColor(4);
		ImGui::PopFont();
		ImGui::End();
		// 恢复样式
		ImGui::PopStyleVar();
	}
	void update(const Timer& timer) {
		x.update(timer.getTime());
	}
	bool is_open() const{
		return open;
	}
	void set_move_time(double time) {
		x.setTotalDuration(time);
	}
	double getX() {
		return x.getX();
	}
} s_menu_gui;

class StatusGui {
private:
	SmoothMove resources_amount{};
	SmoothMove resources_amount_back{};

	void render_bar(ImGuiIO& io, const SmoothMove& f, const SmoothMove& b, const char* title, GLuint tex, int scale) {
		ImGui::Image(tex, ImVec2(30, 30));
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.0f, 0.0f, 0.5f)); // 前景色
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25f, 0.25f, 0.25f, 0.25f));	   // 背景色
		ImVec2 pos = ImGui::GetCursorPos();
		ImGui::ProgressBar(b.getX(), ImVec2(io.DisplaySize.x / 4 - 30, 30), u8"");
		ImGui::PopStyleColor(2); // 恢复颜色设置
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 1.0f, 0.0f, 0.5f)); // 前景色
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25f, 0.25f, 0.25f, 0.0f));	   // 背景色
		ImGui::SetCursorPos(pos);
		ImGui::ProgressBar(f.getX(), ImVec2(io.DisplaySize.x / 4 - 30, 30), u8"");
		ImGui::PopStyleColor(2); // 恢复颜色设置
		ImVec2 text_size = ImGui::CalcTextSize(title);
		ImGui::SetCursorPos(ImVec2(pos.x + 5, pos.y + 30 - text_size.y));
		ImGui::Text(title, (int)(f.getX() * scale));
	}

public:
	StatusGui() {
		// 资源条
		{
			resources_amount.setTotalDuration(0.125);
			resources_amount_back.setTotalDuration(0.5);
		}
	}
	void render_gui(ImGuiIO& io) {
		ImGui::Begin("Status", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBackground);
		ImGui::SetWindowSize(ImVec2(io.DisplaySize.x/4, 200));
		ImGui::SetWindowPos(ImVec2(50, io.DisplaySize.y - 250));

		// 资源条
		render_bar(io, resources_amount, resources_amount_back, u8"资源:%d", TEXTURE::s_image_lightning, 100);
		render_bar(io, resources_amount, resources_amount_back, u8"资源:%d", TEXTURE::s_image_lightning, 100);
		render_bar(io, resources_amount, resources_amount_back, u8"资源:%d", TEXTURE::s_image_lightning, 100);
		render_bar(io, resources_amount, resources_amount_back, u8"资源:%d", TEXTURE::s_image_lightning, 100);

		ImGui::End();
	}
	void update(const Timer& timer) {
		resources_amount.update(timer.getTime());
		resources_amount_back.update(timer.getTime());
	}

	void set_resources_amout(double x, const Timer& timer) {
		resources_amount.newEndPosition(x, timer.getTime());
		resources_amount_back.newEndPosition(x, timer.getTime());
	}
	double get_resources_amount() {
		return resources_amount.getX();
	}

}s_status_gui;

void render_imgui(ImGuiIO& io) {
	if (!s_menu_gui.is_activitied()) {
		int glfw_width, glfw_height;
		glfwGetWindowSize(glfw_win, &glfw_width, &glfw_height);


		ImGui::SetNextWindowBgAlpha(0.25);
		ImGui::Begin("SelectedWeapon", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::SetWindowSize(ImVec2(120, 120));
		ImGui::SetWindowPos(ImVec2(io.DisplaySize.x - 130, io.DisplaySize.y - 130));
		ImGui::SetCursorPos(ImVec2(10, 10));
		switch (s_selected_weapon)
		{
		case NUCLEAR_MISSILE:
			ImGui::Image(TEXTURE::s_image_radioactive, ImVec2(100, 100));
			break;
		case ARMY:
			ImGui::Image(TEXTURE::s_image_attack_target, ImVec2(100, 100));
			break;
		case SCATTER_BOMB:
			ImGui::Image(TEXTURE::s_image_scatter, ImVec2(100, 100));
			break;
		default:
			break;
		}

		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			KeyRelease(GLFW_KEY_SPACE); // 切换武器
		}

		ImGui::End();

		// 设置控件组位置和大小
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(glfw_width, glfw_height));

		// 创建无边框窗口
		ImGui::Begin("##Controls", nullptr,
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoFocusOnAppearing
		);

		ImGui::SetCursorPos(ImVec2(10, 50));

		// 检查窗口是否被点击
		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {

			// 显示格子信息
			if (s_is_selected && s_current_selected_grid[0] == s_selected_gui.grid[0] && s_current_selected_grid[1] == s_selected_gui.grid[1]) {
				s_selected_gui.is_selected = !s_selected_gui.is_selected;
			}
			else {
				s_selected_gui.is_selected = s_is_selected;
				s_selected_gui.grid[0] = s_current_selected_grid[0];
				s_selected_gui.grid[1] = s_current_selected_grid[1];
			}
		}
		// 检查鼠标是否移动
		if (ImGui::IsWindowHovered()) {
			ImVec2 mouse_pos = ImGui::GetMousePos();
			s_mouse_position[0] = mouse_pos.x; s_mouse_position[1] = mouse_pos.y;
		}

		// 在最上方画一个双色进度条来显示已占区块数量和未占区块数量
		ImGui::SetCursorPos(ImVec2(40, 20));

		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 1.0f, 1.0f, 0.5f)); // 前景色
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25f, 0.25f, 0.25f, 0.5f));	   // 背景色
		ImGui::ProgressBar((1 + sin(timer.getTime())) * 0.5, ImVec2(io.DisplaySize.x - 80, 20), u8"");
		ImGui::PopStyleColor(2); // 恢复颜色设置

		ImGui::End();

		s_selected_gui.render_gui(io);
		s_status_gui.render_gui(io);
	}
	s_menu_gui.render_gui(io);
}

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

	
	map_info.bind(0);

	// g_fov
	glUniform1f(glGetUniformLocation(s_map_renderer_program, "g_fov"), -2.0);
	//g_frame_width, g_frame_height
	glUniform1f(glGetUniformLocation(s_map_renderer_program, "g_frame_width"), W);
	glUniform1f(glGetUniformLocation(s_map_renderer_program, "g_frame_height"), H);
	glUniform1f(glGetUniformLocation(s_map_renderer_program, "g_time"), (float)timer.getTime());
	glUniform2i(glGetUniformLocation(s_map_renderer_program, "g_map_size"), 64, 64);


	Camera model_camera;
	model_camera.setPos(0, 2, 0);
	model_camera.setRot(0, -map_rotation.getX(), 0);
	mat4x4 model_mat;
	model_camera.getMat4(model_mat);
	mat4x4_scale(model_mat, model_mat, 0.25);
	glUniformMatrix4fv(glGetUniformLocation(s_map_renderer_program, "g_model_trans_mat"), 1, GL_FALSE, (const GLfloat*)model_mat);

	mat4x4 model_mat_inv_rot;
	model_camera.setPos(0, 2, 0);
	model_camera.setRot(0, map_rotation.getX(), 0);
	model_camera.getMat4(model_mat_inv_rot);
	mat4x4_scale(model_mat_inv_rot, model_mat_inv_rot, 0.25);
	glUniformMatrix4fv(glGetUniformLocation(s_map_renderer_program, "g_model_trans_mat_inv"), 1, GL_FALSE, (const GLfloat*)model_mat_inv_rot);



	mat4x4 g_trans_mat;
	camera.getCamera().getMat4(g_trans_mat);
	mat4x4 g_scale_mat;
	scale_map_camera.getCamera().getMat4(g_scale_mat);
	mat4x4_mul(g_trans_mat, g_trans_mat, g_scale_mat);
	int g_trans_mat_location = glGetUniformLocation(s_map_renderer_program, "g_trans_mat");
	glUniformMatrix4fv(g_trans_mat_location, 1, GL_FALSE, (const GLfloat*)g_trans_mat);




	auto [selected, gridX, gridY] = RegionSelector(-2.0, W, H, g_trans_mat, model_mat, map_info.getWidth(), map_info.getHeight(), map_info.getRegions())(s_mouse_position[0], s_mouse_position[1]);

	s_current_selected_grid[0] = gridX;
	s_current_selected_grid[1] = gridY;
	s_is_selected = selected;

	if (s_selected_gui.is_selected) {
		glUniform2i(glGetUniformLocation(s_map_renderer_program, "g_selected"), s_selected_gui.grid[0], s_selected_gui.grid[1]);
	}
	else {
		glUniform2i(glGetUniformLocation(s_map_renderer_program, "g_selected"), -1, -1);
	}
	if (selected) {
		glUniform2i(glGetUniformLocation(s_map_renderer_program, "g_mouse_selected"), gridX, gridY);
	}
	else {
		glUniform2i(glGetUniformLocation(s_map_renderer_program, "g_mouse_selected"), -1, -1);
	}

	// 核辐射标识
	glUniform4f(glGetUniformLocation(s_map_renderer_program, "g_radioactive_selected"), gridX, gridY, 10, s_selected_weapon == NUCLEAR_MISSILE && selected);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TEXTURE::s_image_radioactive);
	glUniform1i(glGetUniformLocation(s_map_renderer_program, "g_tex_radioactive"), 0);

	// 攻击目标标识
	glUniform3f(glGetUniformLocation(s_map_renderer_program, "g_attack_target"), gridX, gridY, s_selected_weapon == ARMY && selected);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, TEXTURE::s_image_attack_target);
	glUniform1i(glGetUniformLocation(s_map_renderer_program, "g_tex_attack_target"), 1);

	// 散弹炸弹标识
	glUniform4f(glGetUniformLocation(s_map_renderer_program, "g_scatter_target"), gridX, gridY, 20, s_selected_weapon == SCATTER_BOMB && selected);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, TEXTURE::s_image_scatter);
	glUniform1i(glGetUniformLocation(s_map_renderer_program, "g_tex_scatter_target"), 2);

	// 渲染！
	glDrawArrays(GL_QUADS, 0, map_mash.vertexs.size());
	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	map_info.unbind();

	g_main_game_pass_fbo.unbind_frameBuffer();
}

void prepare_render() {
	scale_map_camera.update(timer.getTime());
	scale_map_camera.clampZ(-6, 0,timer.getTime());

	camera.update(timer.getTime());
	camera.clampX(-5, 5, timer.getTime());
	camera.clampZ(-6, 4, timer.getTime());

	map_rotation.update(timer.getTime());

	s_menu_gui.update(timer);
	s_status_gui.update(timer);
}

void render_gaussian_blur() {
	
	{
		g_gaussian_blur_pass_fbo.bind_frameBuffer();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		int W, H;
		float ratio;
		mat4x4 m, p, mvp;

		glfwGetFramebufferSize(glfw_win, &W, &H);

		ratio = W / (float)H;
		int mvp_location = glGetUniformLocation(s_gaussian_blur_program, "MVP");

		mat4x4_identity(m);
		mat4x4_ortho(p, -1, 1, -1.f, 1.f, 1.f, -1.f);
		mat4x4_mul(mvp, p, m);

		glBindBuffer(GL_ARRAY_BUFFER, s_mash.vertex_buffer);
		glUseProgram(s_gaussian_blur_program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);

		glActiveTexture(GL_TEXTURE0);
		g_main_game_pass_fbo.bind_texture();
		glGenerateMipmap(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE1);
		g_gaussian_blur_vertical_pass_fbo.bind_texture();

		glUniform1i(glGetUniformLocation(s_gaussian_blur_program, "g_from_origin"), true);
		glUniform1i(glGetUniformLocation(s_gaussian_blur_program, "g_blur_radius"), 1);
		glUniform1f(glGetUniformLocation(s_gaussian_blur_program, "g_step"), 1);
		glUniform1f(glGetUniformLocation(s_gaussian_blur_program, "g_w_div_h"), ratio);
		glUniform1i(glGetUniformLocation(s_gaussian_blur_program, "g_gaussian"), false);
		glUniform1i(glGetUniformLocation(s_gaussian_blur_program, "g_vertical"), false);

		glDrawArrays(GL_QUADS, 0, s_mash.vertexs.size());
		glUseProgram(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		g_gaussian_blur_pass_fbo.unbind_frameBuffer();
	}
	{
		g_gaussian_blur_vertical_pass_fbo.bind_frameBuffer();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		int W, H;
		float ratio;
		mat4x4 m, p, mvp;

		glfwGetFramebufferSize(glfw_win, &W, &H);

		ratio = W / (float)H;
		int mvp_location = glGetUniformLocation(s_gaussian_blur_program, "MVP");

		mat4x4_identity(m);
		mat4x4_ortho(p, -1, 1, -1.f, 1.f, 1.f, -1.f);
		mat4x4_mul(mvp, p, m);

		glBindBuffer(GL_ARRAY_BUFFER, s_mash.vertex_buffer);
		glUseProgram(s_gaussian_blur_program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);

		glActiveTexture(GL_TEXTURE0);
		g_main_game_pass_fbo.bind_texture();
		glActiveTexture(GL_TEXTURE1);
		g_gaussian_blur_pass_fbo.bind_texture();

		glUniform1i(glGetUniformLocation(s_gaussian_blur_program, "g_from_origin"), false);
		glUniform1i(glGetUniformLocation(s_gaussian_blur_program, "g_blur_radius"), 1);
		glUniform1f(glGetUniformLocation(s_gaussian_blur_program, "g_step"), 1);
		glUniform1f(glGetUniformLocation(s_gaussian_blur_program, "g_w_div_h"), ratio);
		glUniform1i(glGetUniformLocation(s_gaussian_blur_program, "g_gaussian"), false);
		glUniform1i(glGetUniformLocation(s_gaussian_blur_program, "g_vertical"), false);
		glDrawArrays(GL_QUADS, 0, s_mash.vertexs.size());
		glUseProgram(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		g_gaussian_blur_vertical_pass_fbo.unbind_frameBuffer();
	}


	{
		g_gaussian_blur_pass_fbo.bind_frameBuffer();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		int W, H;
		float ratio;
		mat4x4 m, p, mvp;

		glfwGetFramebufferSize(glfw_win, &W, &H);

		ratio = W / (float)H;
		int mvp_location = glGetUniformLocation(s_gaussian_blur_program, "MVP");

		mat4x4_identity(m);
		mat4x4_ortho(p, -1, 1, -1.f, 1.f, 1.f, -1.f);
		mat4x4_mul(mvp, p, m);

		glBindBuffer(GL_ARRAY_BUFFER, s_mash.vertex_buffer);
		glUseProgram(s_gaussian_blur_program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);

		glActiveTexture(GL_TEXTURE0);
		g_main_game_pass_fbo.bind_texture();
		glActiveTexture(GL_TEXTURE1);
		g_gaussian_blur_vertical_pass_fbo.bind_texture();

		glUniform1i(glGetUniformLocation(s_gaussian_blur_program, "g_from_origin"), false);
		glUniform1i(glGetUniformLocation(s_gaussian_blur_program, "g_blur_radius"), 8);
		glUniform1f(glGetUniformLocation(s_gaussian_blur_program, "g_step"), 0.025);
		glUniform1f(glGetUniformLocation(s_gaussian_blur_program, "g_w_div_h"), ratio);
		glUniform1i(glGetUniformLocation(s_gaussian_blur_program, "g_gaussian"), true);
		glUniform1i(glGetUniformLocation(s_gaussian_blur_program, "g_vertical"), true);


		glDrawArrays(GL_QUADS, 0, s_mash.vertexs.size());
		glUseProgram(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		g_gaussian_blur_pass_fbo.unbind_frameBuffer();
	}
	{
		g_gaussian_blur_vertical_pass_fbo.bind_frameBuffer();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		int W, H;
		float ratio;
		mat4x4 m, p, mvp;

		glfwGetFramebufferSize(glfw_win, &W, &H);

		ratio = W / (float)H;
		int mvp_location = glGetUniformLocation(s_gaussian_blur_program, "MVP");

		mat4x4_identity(m);
		mat4x4_ortho(p, -1, 1, -1.f, 1.f, 1.f, -1.f);
		mat4x4_mul(mvp, p, m);

		glBindBuffer(GL_ARRAY_BUFFER, s_mash.vertex_buffer);
		glUseProgram(s_gaussian_blur_program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);

		glActiveTexture(GL_TEXTURE0);
		g_main_game_pass_fbo.bind_texture();
		glActiveTexture(GL_TEXTURE1);
		g_gaussian_blur_pass_fbo.bind_texture();

		glUniform1i(glGetUniformLocation(s_gaussian_blur_program, "g_from_origin"), false);
		glUniform1i(glGetUniformLocation(s_gaussian_blur_program, "g_blur_radius"), 8);
		glUniform1f(glGetUniformLocation(s_gaussian_blur_program, "g_step"), 0.025);
		glUniform1f(glGetUniformLocation(s_gaussian_blur_program, "g_w_div_h"), ratio);
		glUniform1i(glGetUniformLocation(s_gaussian_blur_program, "g_gaussian"), true);
		glUniform1i(glGetUniformLocation(s_gaussian_blur_program, "g_vertical"), false);

		glDrawArrays(GL_QUADS, 0, s_mash.vertexs.size());
		glUseProgram(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		g_gaussian_blur_vertical_pass_fbo.unbind_frameBuffer();
	}
}


void render_final() {
	int W, H;
	float ratio;
	mat4x4 m, p, mvp;
	glfwGetFramebufferSize(glfw_win, &W, &H);
	glViewport(0, 0, W, H);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	ratio = 1;
	int mvp_location = glGetUniformLocation(s_direct_tex_program, "MVP");

	mat4x4_identity(m);
	mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
	mat4x4_mul(mvp, p, m);

	glBindBuffer(GL_ARRAY_BUFFER, s_mash.vertex_buffer);
	glActiveTexture(GL_TEXTURE0);

	if (s_menu_gui.is_activitied()) {
		g_flame_render_pass.render(g_final_mix_pass_fbo.get_texture(), timer, s_mash, s_menu_gui.getX());
		g_flame_render_pass.get_fbo().bind_texture();
	}
	else
		g_final_mix_pass_fbo.bind_texture();

	glUseProgram(s_direct_tex_program);
	glUniform1i(glGetUniformLocation(s_direct_tex_program, "g_pass"), 0);
	glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
	glDrawArrays(GL_QUADS, 0, s_mash.vertexs.size());
	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void render() {
	// HDR
	//glEnable(GL_FRAMEBUFFER_SRGB);
	//glEnable(GL_COLOR_LOGIC_OP);

	render_main_game_pass();
	render_gaussian_blur();

	g_final_mix_pass_fbo.bind_frameBuffer();
	int W, H;
	float ratio;
	mat4x4 m, p, mvp;
	glfwGetFramebufferSize(glfw_win,&W,&H);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	ratio = 1;
	int mvp_location = glGetUniformLocation(s_main_game_pass_program, "MVP");

	mat4x4_identity(m);
	mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
	mat4x4_mul(mvp, p, m);

	glBindBuffer(GL_ARRAY_BUFFER,s_mash.vertex_buffer);
	glActiveTexture(GL_TEXTURE0);
	g_main_game_pass_fbo.bind_texture();
	glActiveTexture(GL_TEXTURE1);
	g_gaussian_blur_vertical_pass_fbo.bind_texture();
	glUseProgram(s_main_game_pass_program);
	glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);

	// 菜单模糊/泛光
	glUniform1f(glGetUniformLocation(s_main_game_pass_program, "g_blur"), 0.05 + 0.95 * s_menu_gui.getX());

	glDrawArrays(GL_QUADS, 0, s_mash.vertexs.size());
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	g_final_mix_pass_fbo.unbind_frameBuffer();

	render_final();

	//glDisable(GL_COLOR_LOGIC_OP);
	//glDisable(GL_FRAMEBUFFER_SRGB);
	//glfwSwapBuffers(glfw_win);

}

void KeyProcess() {
	if (s_menu_gui.is_open()) {
		return;
	}

	float dx = 0, dz = 0;

	float speed = 100 * exp(- 0.25 * scale_map_camera.getZ());

	if (keys[GLFW_KEY_W]) {
		dz = speed;
	}
	if (keys[GLFW_KEY_S]) {
		dz = -speed;
	}
	if (keys[GLFW_KEY_A]) {
		dx = speed;
	}
	if (keys[GLFW_KEY_D]) {
		dx = -speed;
	}

	if (dx != 0 || dz != 0) {
		dx *= timer.dt;
		dz *= timer.dt;
		camera.move(dx, 0, dz, timer.getTime());

	}
	if (keys[GLFW_KEY_R]) {
		// 重置摄像机
		camera.move_to(0, 0, -2, timer.getTime());
		camera.rotate_to(0, 0, -1.5, timer.getTime());
		scale_map_camera.move_to(0, 0, -6, timer.getTime());
		camera.rotate_to(0, 0, -1.5 / (1 + 2 * exp(-0.05 * -6 * -6)), timer.getTime());
		map_rotation.newEndPosition(-(exp(-0.01 * pow(-6 * -6, 2))), timer.getTime());
	}
}

void KeyRelease(int key) {
	if(key == GLFW_KEY_ESCAPE)
		s_menu_gui.open_gui(!s_menu_gui.is_open(), timer);
	
	if (s_menu_gui.is_open()) {
		return;
	}
	switch (key)
	{
	case GLFW_KEY_SPACE:
		s_selected_weapon = (SelectedWeapon)((s_selected_weapon + 1) % 3);
		break;
	case GLFW_KEY_F:
		s_status_gui.set_resources_amout(fmod(s_status_gui.get_resources_amount() + 0.1,1), timer);
	default:
		break;
	}
}


bool compileShaders() {
	DEBUG::DebugOutput("Compiling Shaders");
	s_main_game_pass_program = CompileShader(main_game_pass_vert, main_game_pass_frag, nullptr, &vertex_shader, &fragment_shader, nullptr);
	if (s_main_game_pass_program == -1) return false;
	s_normal_gl_program = CompileShader(normal_gl_vert, normal_gl_frag, nullptr, &normal_gl_vertex_shader, &normal_gl_fragment_shader, nullptr);
	if (s_normal_gl_program == -1) return false;
	s_map_renderer_program = CompileShader(map_renderer_vert, map_renderer_frag, nullptr, &map_renderer_vertex_shader, &map_renderer_fragment_shader, nullptr);
	if (s_map_renderer_program == -1) return false;
	s_gaussian_blur_program = CompileShader(gaussian_blur_vert, gaussian_blur_frag, nullptr, &gaussian_blur_vertex_shader, &gaussian_blur_fragment_shader, nullptr);
	if (s_gaussian_blur_program == -1) return false;
	s_direct_tex_program = CompileShader(direct_tex_pass_vert, direct_tex_pass_frag, nullptr, &direct_tex_vertex_shader, &direct_tex_fragment_shader, nullptr);
	if (s_direct_tex_program == -1) return false;
	DEBUG::DebugOutput("Shaders Compiled");
}

float randfloat() {
	return (float)rand() / RAND_MAX;
}

void init() {


	scale_map_camera.setMoveDuration(0.25);
	scale_map_camera.setPos(0, 0, -32,timer.getTime());
	camera.setMoveDuration(0.5);
	camera.setRotateDuration(0.25);
	camera.rotate(0, 0, -1.5, timer.getTime());
	camera.move(0, 0, -2, timer.getTime());
	map_rotation.setStartPosition(-1, timer.getTime());
	map_rotation.setTotalDuration(0.25);

	glfwScrollCallback(glfw_win, 0, 0);

	DEBUG::DebugOutput("Building meshes..");

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



	for (int i{}; i < 64; i++) {
		for (int j{}; j < 64; j++) {
			RegionData region;
			region.cell_center_x = randfloat();
			region.cell_center_y = randfloat();
			region.identity = (int)(i * i + j * j < 400) + (int)(i * i + j * j < 200);
			region.padding_1 = 0;
			map_info.setRegion(i, j, region);
		}
	}
	map_info.create_ssbo();
	map_info.update();



	DEBUG::DebugOutput("SSBO Created");
	DEBUG::DebugOutput("Loading Textures...");
	TEXTURE::s_image_radioactive = LoadPNG("resources/textures/radioactivity.png");
	TEXTURE::s_image_attack_target = LoadPNG("resources/textures/target.png");
	TEXTURE::s_image_scatter = LoadPNG("resources/textures/scatter.png");
	TEXTURE::s_image_lightning = LoadPNG("resources/textures/lightning.png");
	DEBUG::DebugOutput("Textures Loaded");
	timer.setTime(glfwGetTime());

}

void destroy() {
	if (s_main_game_pass_program != -1) glDeleteProgram(s_main_game_pass_program);
	if (s_normal_gl_program != -1) glDeleteProgram(s_normal_gl_program);
	if (s_map_renderer_program != -1) glDeleteProgram(s_map_renderer_program);
	if (s_gaussian_blur_program != -1) glDeleteProgram(s_gaussian_blur_program);
	if (s_direct_tex_program != -1) glDeleteProgram(s_direct_tex_program);
	if (vertex_shader != -1) glDeleteShader(vertex_shader);
	if (fragment_shader != -1) glDeleteShader(fragment_shader);
	if (normal_gl_vertex_shader != -1) glDeleteShader(normal_gl_vertex_shader);
	if (normal_gl_fragment_shader != -1) glDeleteShader(normal_gl_fragment_shader);
	if (map_renderer_vertex_shader != -1) glDeleteShader(map_renderer_vertex_shader);
	if (map_renderer_fragment_shader != -1) glDeleteShader(map_renderer_fragment_shader);
	if (gaussian_blur_vertex_shader != -1) glDeleteShader(gaussian_blur_vertex_shader);
	if (gaussian_blur_fragment_shader != -1) glDeleteShader(gaussian_blur_fragment_shader);
	if (direct_tex_vertex_shader != -1) glDeleteShader(direct_tex_vertex_shader);
	if (direct_tex_fragment_shader != -1) glDeleteShader(direct_tex_fragment_shader);
	
	g_gaussian_blur_pass_fbo.release();
	g_gaussian_blur_vertical_pass_fbo.release();
	g_main_game_pass_fbo.release();
	g_final_mix_pass_fbo.release();
	g_flame_render_pass.release();

	if (TEXTURE::s_image_radioactive != GLFW_INVALID_VALUE) {
		glDeleteTextures(1, &TEXTURE::s_image_radioactive);
	}
	if (TEXTURE::s_image_attack_target != GLFW_INVALID_VALUE) {
		glDeleteTextures(1, &TEXTURE::s_image_attack_target);
	}
	if (TEXTURE::s_image_scatter != GLFW_INVALID_VALUE) {
		glDeleteTextures(1, &TEXTURE::s_image_scatter);
	}
	if (TEXTURE::s_image_lightning != GLFW_INVALID_VALUE) {
		glDeleteTextures(1, &TEXTURE::s_image_lightning);
	}
}

int main() {
	if (!glfwInit()) {
		println("Failed to initialize glfw"); return 0;
	}
	glfwSetErrorCallback((GLFWerrorfun)glfwErrorCallBack);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	GLFWmonitor* primary = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primary);

	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

	glfw_win = glfwCreateWindow(mode->width, mode->height, "MiniWar", primary, NULL);

	glfwSetKeyCallback(glfw_win, (GLFWkeyfun)glfwKeyCallBack);
	glfwSetCursorPosCallback(glfw_win, (GLFWcursorposfun)glfwMouseCallback);
	glfwSetWindowSizeCallback(glfw_win, (GLFWwindowsizefun)glfwWindowSizeCallback);
	glfwSetScrollCallback(glfw_win, (GLFWscrollfun)glfwScrollCallback);

	glfwMakeContextCurrent(glfw_win);

	glfwSwapInterval(1);
	if (!glfw_win) {
		println("Failed to create window"); return 0;
	}



	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;

	

	UIFonts::default_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 32.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
	UIFonts::large_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 48.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
	UIFonts::menu_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 64.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(glfw_win, true);
	const char* glsl_version = "#version 330";
	ImGui_ImplOpenGL3_Init(glsl_version);

	int err = glewInit();

	if (err) {
		println("Failed to initialize glew");
		println((char*)glewGetErrorString(err));
		goto destroy;
	}
	GLint float_framebuffer_supported;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &float_framebuffer_supported);
	if (!float_framebuffer_supported) {
		DEBUG::DebugOutput("ARB_color_buffer_float not supported");
	}
	else {
		// 启用扩展
		glewExperimental = GL_TRUE;
		if (GLEW_ARB_color_buffer_float) {
			DEBUG::DebugOutput("Activate ARB_color_buffer_float");
			glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);
			glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);
			glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);
		}
	}

#ifdef _DEBUG
	glDebugMessageCallback((GLDEBUGPROC)debugProc, 0);
	glEnable(GL_DEBUG_CALLBACK_FUNCTION);
#endif // DEBUG

	DEBUG::DebugOutput("OpenGL Version", (std::string)(char*)glGetString(GL_VERSION));
	DEBUG::DebugOutput("GLSL Version", (std::string)(char*)glGetString(GL_SHADING_LANGUAGE_VERSION));


	int width, height;
	glfwGetFramebufferSize(glfw_win, &width, &height);
	g_main_game_pass_fbo = FragmentBuffer(width, height, GL_RGBA16F, true);
	g_final_mix_pass_fbo = FragmentBuffer(width, height, GL_RGBA16F);
	g_gaussian_blur_pass_fbo = FragmentBuffer(width, height, GL_RGBA16F);
	g_gaussian_blur_vertical_pass_fbo = FragmentBuffer(width, height, GL_RGBA16F);
	g_flame_render_pass = FlameRenderPass(width, height, GL_RGBA16F);
	g_flame_render_pass.init();


	if (!compileShaders()) goto destroy;

	init();




	while (!glfwWindowShouldClose(glfw_win))
	{

		glfwPollEvents();
		if (glfwGetWindowAttrib(glfw_win, GLFW_ICONIFIED) != 0)
		{
			ImGui_ImplGlfw_Sleep(10);
			continue;
		}
		if (s_pause_rendering) {
			ImGui_ImplGlfw_Sleep(10);
			continue;
		}
		timer.setTime(glfwGetTime());
		prepare_render();
		KeyProcess();
		render();


		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		render_imgui(io);

		// Rendering
		glDebugMessageCallback(0, 0);
		ImGui::Render();
		glDebugMessageCallback((GLDEBUGPROC)debugProc, 0);


		glDebugMessageCallback(0, 0);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glDebugMessageCallback((GLDEBUGPROC)debugProc, 0);

		glfwSwapBuffers(glfw_win);
	}
destroy:

	destroy();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

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
		//case GLFW_KEY_ESCAPE:
			//glfwSetWindowShouldClose(glfw_win, GLFW_TRUE); break;
		default:
			break;

		}
		return;
	}
	// 按键松开
	if (action == GLFW_RELEASE) {
		switch (key)
		{
		//case GLFW_KEY_ESCAPE:
			//break;
		default:
			KeyRelease(key);
		}
	}
}
void glfwMouseCallback(GLFWwindow* window, double xpos, double ypos) {

}

// 滚轮事件
void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	if (s_menu_gui.is_open()) {
		return;
	}
	scale_map_camera.move(0, 0, yoffset, timer.getTime());
	camera.rotate_to(0, 0, -1.5 /(1 + 2 * exp(-0.05 * scale_map_camera.getZ() * scale_map_camera.getZ())), timer.getTime());
	map_rotation.newEndPosition(-(exp(-0.01 * pow(scale_map_camera.getZ() * scale_map_camera.getZ(),2))),timer.getTime());
}

void glfwWindowSizeCallback(GLFWwindow* window, int width, int height) {
	try {
		glViewport(0, 0, width, height);
		g_main_game_pass_fbo.resize(width, height, true);
		g_gaussian_blur_pass_fbo.resize(width, height);
		g_gaussian_blur_vertical_pass_fbo.resize(width, height);
		g_final_mix_pass_fbo.resize(width, height);
		g_flame_render_pass.get_fbo().resize(width, height);
		s_pause_rendering = false;
	}
	catch (const char* str) {
		s_pause_rendering = true;
	}
}