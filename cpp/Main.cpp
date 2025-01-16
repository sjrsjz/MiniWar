
#include "../include/GL/glew.h"
#include "../include/GLFW/glfw3.h"
#include "../include/GLFW/glfw3native.h"
#include "../include/linmath.h"

#include "../header/output.h"
#include "../header/shader.h"
#include "../header/mesh.h"
#include "../header/Camera.h"
#include "../header/Timer.h"

#include "../header/debug.h"
#include "../header/utils/RegionSSBOBuffer.h"
#include "../header/utils/RegionData.h"
#include "../header/Logic/GameEffect.h"
#include "../header/globals.h"

#include "../header/utils/FloatBuffer.h"
#include "../header/utils/RegionSelector.h"
#include "../header/utils/CoordTranslate.h"

#include "../include/imgui/imgui.h"
#include "../include/imgui/imgui_impl_glfw.h"
#include "../include/imgui/imgui_impl_opengl3.h"
#include "../include/bass/bass.h"

#include "../header/utils/ImageLoader.h"

// Engine
#include "../header/Engine.h"


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
#include "../shaders/points_renderer.vert"
#include "../shaders/points_renderer.frag"
#include "../header/points.h"


#include <cmath>
#include <mutex>

namespace FBO {
	static FragmentBuffer g_final_mix_pass_fbo;
	static FragmentBuffer g_main_game_pass_fbo;
	static FragmentBuffer g_gaussian_blur_pass_fbo;
	static FragmentBuffer g_gaussian_blur_vertical_pass_fbo;
	static FlameRenderPass g_flame_render_pass;
	static FragmentBuffer g_point_render_pass_fbo;
}
namespace RENDERER
{
	static Timer timer(0);	
}

namespace MAP {
	static Mesh s_mesh;
	static SmoothCamera camera;
	static SmoothCamera scale_map_camera; // 缩放摄像机
	static Mesh map_mesh;
	static RegionSSBOBuffer map_info;
	static SmoothMove map_rotation;
	static const float map_plane_y = -0.62f;
}


namespace PASS_MAIN {
	static GLuint vertex_shader, fragment_shader, s_main_game_pass_program;
}

namespace PASS_NORMAL {
	static GLuint normal_gl_vertex_shader, normal_gl_fragment_shader, s_normal_gl_program;
}

namespace PASS_POINT {
	static GLuint points_vertex_shader, points_fragment_shader, s_points_program;
	static PointRenderer point_renderer;
}


namespace PASS_DIRECT_TEX {
	static GLuint direct_tex_vertex_shader, direct_tex_fragment_shader, s_direct_tex_program;
}

namespace PASS_MAP_RENDER {
	static GLuint map_renderer_vertex_shader, map_renderer_fragment_shader, s_map_renderer_program;
}

namespace PASS_BLOOM {
	static GLuint gaussian_blur_vertex_shader, gaussian_blur_fragment_shader, s_gaussian_blur_program;
}

namespace INTERACTIVE {
	static bool keys[512];
	static vec3 s_mouse_position;
	static int s_current_selected_grid[2] = { -1,-1 };
	static bool s_is_selected = false;
}

namespace MODEL {
	namespace MISSLE {
		static Mesh missle_mesh;
		static GLuint missle_texture;
	}

}

namespace GAMESOUND {
#ifdef _WIN32
#define SOUND_STR(str) L##str
#define SOUND_CHAR wchar_t
#else
#define SOUND_STR(str) str
#define SOUND_CHAR char
#endif

	static const SOUND_CHAR* s_sound_background[] = {
		SOUND_STR("./resources/sounds/background/background1.mp3"),
		SOUND_STR("./resources/sounds/background/background2.mp3"),
		SOUND_STR("./resources/sounds/background/background3.mp3"),
	};

	static const SOUND_CHAR* s_sound_click[] = {
		SOUND_STR("./resources/sounds/interact/click1.mp3")
	};

	static const SOUND_CHAR* s_sound_popup[] = {
		SOUND_STR("./resources/sounds/interact/popup1.mp3")
	};

	static const SOUND_CHAR* s_sound_nuclear_launch[] = {
		SOUND_STR("./resources/sounds/interact/nuclear_launch.mp3")
	};

	static const SOUND_CHAR* s_sound_error[] = {
		SOUND_STR("./resources/sounds/interact/error.mp3")
	};

	static const SOUND_CHAR* s_sound_bomb_explosion[] = {
		SOUND_STR("./resources/sounds/interact/bomb_explosion.mp3")
	};

	static int s_sound_background_idx = 0;
	static HSTREAM s_background_stream;

	static const int MAX_SOUND_CHANNELS = 64;  // 最大同时播放数
	static HSTREAM s_sound_streams[MAX_SOUND_CHANNELS];
	static int s_current_sound_idx = 0;

	void init_bass() {
		// 先确保BASS已关闭
		BASS_Free();

		// 初始化BASS，使用默认设备
		if (!BASS_Init(-1,         // 默认设备
			44100,      // 采样率
			BASS_DEVICE_STEREO,  // 立体声
			0,          // 窗口句柄
			NULL        // 不使用CLSID
		)) {
			int error = BASS_ErrorGetCode();
			DEBUGOUTPUT("BASS Init Error: ", error);
			return;
		}

		// 设置全局配置
		BASS_SetConfig(BASS_CONFIG_BUFFER, 100);  // 设置缓冲区大小
		BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 10);  // 设置更新周期

		DEBUGOUTPUT("BASS Init Success");


		s_sound_background_idx = rand() % (sizeof(s_sound_background) / sizeof(s_sound_background[0]));
	}
	void play_background() {
		s_background_stream = BASS_StreamCreateFile(FALSE,
			s_sound_background[s_sound_background_idx],
			0, 0,
			BASS_STREAM_AUTOFREE | BASS_ASYNCFILE);
		s_sound_background_idx = (s_sound_background_idx + 1) % (sizeof(s_sound_background) / sizeof(s_sound_background[0]));
		if (s_background_stream) {
			BASS_ChannelStart(s_background_stream);
			BASS_ChannelPlay(s_background_stream, TRUE);
		}
	}
	void set_background_volume(float volume) {
		BASS_ChannelSetAttribute(s_background_stream, BASS_ATTRIB_VOL, volume);
	}
	void check_if_need_play_next() {
		if (!BASS_ChannelIsActive(s_background_stream)) {
			DEBUGOUTPUT("Background Music End, Play Next");
			BASS_StreamFree(s_background_stream);
			play_background();
		}
	}
	void release() {
		// 释放背景音乐
		if (s_background_stream) {
			BASS_StreamFree(s_background_stream);
		}
		// 释放所有音效
		for (int i = 0; i < MAX_SOUND_CHANNELS; i++) {
			if (s_sound_streams[i]) {
				BASS_StreamFree(s_sound_streams[i]);
			}
		}
		BASS_Free();
	}
	void play_click_sound() {
		// 释放已经播放完的音效
		if (s_sound_streams[s_current_sound_idx] &&
			BASS_ChannelIsActive(s_sound_streams[s_current_sound_idx]) == BASS_ACTIVE_STOPPED) {
			BASS_StreamFree(s_sound_streams[s_current_sound_idx]);
		}

		// 创建新的音效流
		int rand_idx = rand() % (sizeof(s_sound_click) / sizeof(s_sound_click[0]));
		s_sound_streams[s_current_sound_idx] = BASS_StreamCreateFile(
			FALSE, s_sound_click[rand_idx], 0, 0, 0);

		if (s_sound_streams[s_current_sound_idx]) {
			BASS_ChannelSetAttribute(s_sound_streams[s_current_sound_idx],
				BASS_ATTRIB_VOL, 1.0f);  // 设置音量
			BASS_ChannelPlay(s_sound_streams[s_current_sound_idx], FALSE);
		}

		// 循环使用通道
		s_current_sound_idx = (s_current_sound_idx + 1) % MAX_SOUND_CHANNELS;
	}
	void play_popup_sound() {
		// 释放已经播放完的音效
		if (s_sound_streams[s_current_sound_idx] &&
			BASS_ChannelIsActive(s_sound_streams[s_current_sound_idx]) == BASS_ACTIVE_STOPPED) {
			BASS_StreamFree(s_sound_streams[s_current_sound_idx]);
		}

		// 创建新的音效流
		int rand_idx = rand() % (sizeof(s_sound_popup) / sizeof(s_sound_popup[0]));
		s_sound_streams[s_current_sound_idx] = BASS_StreamCreateFile(
			FALSE, s_sound_popup[rand_idx], 0, 0, 0);

		if (s_sound_streams[s_current_sound_idx]) {
			BASS_ChannelSetAttribute(s_sound_streams[s_current_sound_idx],
				BASS_ATTRIB_VOL, 1.0f);  // 设置音量
			BASS_ChannelPlay(s_sound_streams[s_current_sound_idx], FALSE);
		}

		// 循环使用通道
		s_current_sound_idx = (s_current_sound_idx + 1) % MAX_SOUND_CHANNELS;
	}

	void play_nuclear_launch_sound() {
		// 释放已经播放完的音效
		if (s_sound_streams[s_current_sound_idx] &&
			BASS_ChannelIsActive(s_sound_streams[s_current_sound_idx]) == BASS_ACTIVE_STOPPED) {
			BASS_StreamFree(s_sound_streams[s_current_sound_idx]);
		}

		// 创建新的音效流
		int rand_idx = rand() % (sizeof(s_sound_nuclear_launch) / sizeof(s_sound_nuclear_launch[0]));
		s_sound_streams[s_current_sound_idx] = BASS_StreamCreateFile(
			FALSE, s_sound_nuclear_launch[rand_idx], 0, 0, 0);

		if (s_sound_streams[s_current_sound_idx]) {
			BASS_ChannelSetAttribute(s_sound_streams[s_current_sound_idx],
				BASS_ATTRIB_VOL, 1.0f);  // 设置音量
			BASS_ChannelPlay(s_sound_streams[s_current_sound_idx], FALSE);
		}

		// 循环使用通道
		s_current_sound_idx = (s_current_sound_idx + 1) % MAX_SOUND_CHANNELS;
	}

	void play_error_sound() {
		// 释放已经播放完的音效
		if (s_sound_streams[s_current_sound_idx] &&
			BASS_ChannelIsActive(s_sound_streams[s_current_sound_idx]) == BASS_ACTIVE_STOPPED) {
			BASS_StreamFree(s_sound_streams[s_current_sound_idx]);
		}
		// 创建新的音效流
		int rand_idx = rand() % (sizeof(s_sound_error) / sizeof(s_sound_error[0]));
		s_sound_streams[s_current_sound_idx] = BASS_StreamCreateFile(
			FALSE, s_sound_error[rand_idx], 0, 0, 0);

		if (s_sound_streams[s_current_sound_idx]) {
			BASS_ChannelSetAttribute(s_sound_streams[s_current_sound_idx],
				BASS_ATTRIB_VOL, 1.0f);  // 设置音量
			BASS_ChannelPlay(s_sound_streams[s_current_sound_idx], FALSE);
		}

		// 循环使用通道
		s_current_sound_idx = (s_current_sound_idx + 1) % MAX_SOUND_CHANNELS;
	}
	void play_bomb_explosion_sound() {
		// 释放已经播放完的音效
		if (s_sound_streams[s_current_sound_idx] &&
			BASS_ChannelIsActive(s_sound_streams[s_current_sound_idx]) == BASS_ACTIVE_STOPPED) {
			BASS_StreamFree(s_sound_streams[s_current_sound_idx]);
		}
		// 创建新的音效流
		int rand_idx = rand() % (sizeof(s_sound_bomb_explosion) / sizeof(s_sound_bomb_explosion[0]));
		s_sound_streams[s_current_sound_idx] = BASS_StreamCreateFile(
			FALSE, s_sound_bomb_explosion[rand_idx], 0, 0, 0);

		if (s_sound_streams[s_current_sound_idx]) {
			BASS_ChannelSetAttribute(s_sound_streams[s_current_sound_idx],
				BASS_ATTRIB_VOL, 1.0f);  // 设置音量
			BASS_ChannelPlay(s_sound_streams[s_current_sound_idx], FALSE);
		}

		// 循环使用通道
		s_current_sound_idx = (s_current_sound_idx + 1) % MAX_SOUND_CHANNELS;
	}
#undef SOUND_STR
#undef SOUND_CHAR
}

namespace GAMESTATUS {
	static bool s_in_game = false;
	static bool s_enable_control = false;
}

namespace LEVELDATA {
	static const char* s_levels[] = { u8"简单(16x16)", u8"普通(32x32)", u8"困难(64x64)", u8"噩梦(128x128)"};
	static enum Level {
		EASY,
		NORMAL,
		HARD,
		CRAZY,
	}s_selected_level;
}

namespace UIFonts {
	static ImFont* default_font;
	static ImFont* menu_font;
	static ImFont* large_font;
}
namespace TEXTURE {
	static GLuint s_image_radioactive;
	static GLuint s_image_attack_target;
	static GLuint s_image_scatter;
	static GLuint s_image_lightning;
	static GLuint s_image_guard;
	static GLuint s_image_forbid;
	static GLuint s_image_building_icon;
}

static GLFWwindow* glfw_win;
static float s_dpi_scale = 1.0f;
static bool s_pause_rendering = false;
static int s_scatter_bomb_range = 1;
static int s_nuclear_missile_level = 0;
extern bool g_game_over;
extern bool g_game_stop;

static enum SelectedWeapon {
	NONE, // 空
	NUCLEAR_MISSILE, // 核导弹
	ARMY, // 军队
	SCATTER_BOMB, // 散弹炸弹
} s_selected_weapon;




void glfwErrorCallBack(int error, const char* str);
void glfwKeyCallBack(GLFWwindow* window, int key, int scanmode, int action, int mods);
void glfwMouseCallback(GLFWwindow* window, double xpos, double ypos);
void glfwWindowSizeCallback(GLFWwindow* window, int width, int height);
void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void KeyProcess();
void KeyRelease(int key);
void get_screen_position(float x, float y, float z, float& screen_x, float& screen_y);

inline ImVec2 operator+(const ImVec2& a, const ImVec2& b) {
	return ImVec2(a.x + b.x, a.y + b.y);
}

inline ImVec2 operator-(const ImVec2& a, const ImVec2& b) {
	return ImVec2(a.x - b.x, a.y - b.y);
}

inline ImVec2 operator*(const ImVec2& v, float f) {
	return ImVec2(v.x * f, v.y * f);
}

inline ImVec2 normalize(const ImVec2& v) {
	float len = sqrt(v.x * v.x + v.y * v.y);
	return ImVec2(v.x / len, v.y / len);
}

struct LevelConfig {
	int map_width = 16;
	int map_height = 16;

};

static class TechTreeGui {
	struct TechNode {
		const char* name;
		bool unlocked;
		std::vector<int> dependencies;  // 依赖节点的索引
		ImVec2 pos;                    // 节点位置
		GLuint tex;                   // 节点纹理
		std::function<void()> on_click{};  // 点击事件
	};

	SmoothMove tree_offset_X{};
	SmoothMove tree_offset_Y{};

	class TechTree {
	public:
		std::vector<TechNode> nodes;
		bool align_center = true;
		void draw(ImGuiIO& io, double alpha, ImVec2 tree_offset) {
			ImGui::Begin("Tech Tree");

			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			ImVec2 offset = tree_offset + (align_center ? ImVec2(io.DisplaySize.x / 2, io.DisplaySize.y / 2) : ImGui::GetCursorScreenPos());

			// 绘制连线
			for (size_t i = 0; i < nodes.size(); i++) {
				for (int dep : nodes[i].dependencies) {
					auto text_size_1 = ImGui::CalcTextSize(nodes[i].name);
					auto text_size_2 = ImGui::CalcTextSize(nodes[dep].name);
					float r1 = fmaxf(text_size_1.x, text_size_1.y) / 2 + 10;
					float r2 = fmaxf(text_size_2.x, text_size_2.y) / 2 + 10;
					ImVec2 p1 = offset + (nodes[dep].pos * s_dpi_scale + normalize(nodes[i].pos - nodes[dep].pos) * r2);
					ImVec2 p2 = offset + (nodes[i].pos * s_dpi_scale + normalize(nodes[dep].pos - nodes[i].pos) * r1);
					if (nodes[dep].unlocked && nodes[i].unlocked)
						draw_list->AddLine(p1 , p2, IM_COL32(100, 255, 100, 128 * alpha), 5.0f);
					else
						draw_list->AddLine(p1, p2, IM_COL32(200, 200, 200, 128 * alpha), 5.0f);
				}
			}

			// 绘制节点
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, alpha));
			for (auto& node : nodes) {
				ImVec2 p0 = offset + node.pos * s_dpi_scale;
				ImGui::SetCursorScreenPos(p0);

				ImU32 node_color = node.unlocked ?
					IM_COL32(100, 255, 100, 64 * alpha) :
					IM_COL32(255, 100, 100, 64 * alpha);
				// 居中显示名称
				auto text_size = ImGui::CalcTextSize(node.name);

				draw_list->AddCircleFilled(
					p0,
					fmaxf(text_size.x, text_size.y) / 2 + 10,
					node_color
				);
				float L = fmaxf(text_size.x, text_size.y) + 20;
				ImGui::SetCursorScreenPos(p0 - ImVec2(L / 4, L / 4));
				ImGui::Image(node.tex,
					ImVec2(L / 2, L / 2),
					ImVec2(0, 0),                    // UV coordinates - 左上
					ImVec2(1, 1),                    // UV coordinates - 右下
					ImVec4(1.0f, 1.0f, 1.0f, 0.25f * alpha)  // 颜色调制 (这里设置50%透明)
				);
				ImGui::SetCursorScreenPos(p0 - ImVec2(L / 2, L / 2));
				if (ImGui::InvisibleButton(node.name, ImVec2(L, L))) {

					// 处理点击
					// 如果父节点都解锁了，那么解锁当前节点
					bool unlock = true;
					for (int dep : node.dependencies) {
						if (!nodes[dep].unlocked) {
							unlock = false;
							break;
						}
					}
					if (unlock) {
						GAMESOUND::play_click_sound();
						node.on_click();
					}
				}
				ImGui::SetCursorScreenPos(
					p0 - ImVec2(text_size.x / 2, text_size.y / 2)
				);
				ImGui::Text("%s", node.name);
			}
			ImGui::PopStyleColor();
			ImGui::End();
		}
	};

	TechTree tree{};
	SmoothMove show{};
public:
	void init_tech_tree_gui() {
		tree.nodes.clear();
		show.set_total_duration(0.25);
		show.set_start_position(0, 0);
		tree_offset_X.set_total_duration(0.125);
		tree_offset_Y.set_total_duration(0.125);
		tree_offset_X.set_start_position(0, 0);
		tree_offset_Y.set_start_position(0, 0);

		tree.nodes.push_back({ u8"发电站", true, {} , ImVec2(-450,400), TEXTURE::s_image_lightning,[]() {} });
		tree.nodes.push_back({ u8"炼油厂", true, {} , ImVec2(-500,200), TEXTURE::s_image_lightning,[]() {} });
		tree.nodes.push_back({ u8"炼钢厂", true, {} , ImVec2(-600,0), TEXTURE::s_image_lightning,[]() {} });
		tree.nodes.push_back({ u8"民生工厂", true, {} , ImVec2(-500,-200), TEXTURE::s_image_lightning,[]() {} });
		tree.nodes.push_back({ u8"军事工厂", true, {} , ImVec2(-450,-400), TEXTURE::s_image_guard,[]() {} });

		tree.nodes.push_back({ u8"研究所",false, {0, 1, 2, 3, 4, 25}, ImVec2(0,0), TEXTURE::s_image_guard,[]() {
				push_input({Operator::SetResearch});
			} });

		tree.nodes.push_back({ u8"发电站二级", false, {5}, ImVec2(300,400), TEXTURE::s_image_lightning ,[]() {
				push_input({Operator::PowerStationUpLevel});
			} });
		tree.nodes.push_back({ u8"炼油厂二级", false, {5}, ImVec2(400,200), TEXTURE::s_image_lightning ,[]() {
				push_input({Operator::RefineryUpLevel});
			} });
		tree.nodes.push_back({ u8"炼钢厂二级", false, {5}, ImVec2(500,0), TEXTURE::s_image_lightning ,[]() {
				push_input({Operator::SteelFactoryUpLevel});
			} });
		tree.nodes.push_back({ u8"民生工厂二级", false, {5}, ImVec2(400,-200), TEXTURE::s_image_lightning ,[]() {
				push_input({Operator::CivilFactoryUpLevel});
			} });
		tree.nodes.push_back({ u8"军事工厂二级", false, {5}, ImVec2(300,-400), TEXTURE::s_image_guard ,[]() {
				push_input({Operator::MilitaryFactoryUpLevel});
			} });

		// 三级
		tree.nodes.push_back({ u8"发电站三级", false, {6}, ImVec2(600,400), TEXTURE::s_image_lightning ,[]() {
				push_input({Operator::PowerStationUpLevel});
			} });
		tree.nodes.push_back({ u8"炼油厂三级", false, {7}, ImVec2(700,200), TEXTURE::s_image_lightning ,[]() {
				push_input({Operator::RefineryUpLevel});
			} });
		tree.nodes.push_back({ u8"炼钢厂三级", false, {8}, ImVec2(800,0), TEXTURE::s_image_lightning ,[]() {
				push_input({Operator::SteelFactoryUpLevel});
			} });
		tree.nodes.push_back({ u8"民生工厂三级", false, {9}, ImVec2(700,-200), TEXTURE::s_image_lightning ,[]() {
				push_input({Operator::CivilFactoryUpLevel});
			} });
		tree.nodes.push_back({ u8"军事工厂三级", false, {10}, ImVec2(600,-400), TEXTURE::s_image_guard ,[]() {
				push_input({Operator::MilitaryFactoryUpLevel});
			} });

		// 导弹
		tree.nodes.push_back({ u8"短程核导弹", false, {5}, ImVec2(0,300), TEXTURE::s_image_radioactive ,[]() {
				push_input({Operator::Weapon0UpLevel});
			} });
		tree.nodes.push_back({ u8"中程核导弹", false, {16}, ImVec2(300,600), TEXTURE::s_image_radioactive ,[]() {
				push_input({Operator::Weapon1UpLevel});
			} });
		tree.nodes.push_back({ u8"长程核导弹", false, {17}, ImVec2(100,1000), TEXTURE::s_image_radioactive ,[]() {
				push_input({Operator::Weapon2UpLevel});
			} });

		// 短程2、3级
		tree.nodes.push_back({ u8"短程二级", false, {16}, ImVec2(-100,450), TEXTURE::s_image_radioactive ,[]() {
				push_input({Operator::Weapon0UpLevel});
			} });
		tree.nodes.push_back({ u8"短程三级", false, {19}, ImVec2(-100,600), TEXTURE::s_image_radioactive ,[]() {
				push_input({Operator::Weapon0UpLevel});
			} });

		// 中程2、3级
		tree.nodes.push_back({ u8"中程二级", false, {17}, ImVec2(300,850), TEXTURE::s_image_radioactive ,[]() {
				push_input({Operator::Weapon1UpLevel});
			} });
		tree.nodes.push_back({ u8"中程三级", false, {21}, ImVec2(500,850), TEXTURE::s_image_radioactive ,[]() {
				push_input({Operator::Weapon1UpLevel});
			} });

		// 长程2、3级
		tree.nodes.push_back({ u8"长程二级", false, {18}, ImVec2(100,1200), TEXTURE::s_image_radioactive ,[]() {
				push_input({Operator::Weapon2UpLevel});
			} });
		tree.nodes.push_back({ u8"长程三级", false, {23}, ImVec2(300,1200), TEXTURE::s_image_radioactive ,[]() {
				push_input({Operator::Weapon2UpLevel});
			} });

		// 军队
		tree.nodes.push_back({ u8"军队", true, {}, ImVec2(-200,-400), TEXTURE::s_image_guard ,[]() {} });
		tree.nodes.push_back({ u8"军队二级", false, {5}, ImVec2(0,-300), TEXTURE::s_image_guard ,[]() {
				push_input({Operator::ArmyUpLevel});
			} });
		tree.nodes.push_back({ u8"军队三级", false, {26}, ImVec2(-100,-500), TEXTURE::s_image_guard ,[]() {
				push_input({Operator::ArmyUpLevel});
			} });

	}
	bool is_active() {
		return show.x() > 1e-3;
	}
	void render_gui(ImGuiIO& io) {
		if (!is_active()) return;

		ImGui::SetNextWindowBgAlpha(0.75 * show.x());
		ImGui::Begin("Tech Tree", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		ImGui::SetWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
		ImGui::SetWindowPos(ImVec2(0, 0) * s_dpi_scale);

		tree.draw(io, show.x(), ImVec2(1000 * tree_offset_X.x(), 1000 * tree_offset_Y.x()) * s_dpi_scale);

		ImGui::End();
	}

	void update(const Timer& timer) {
		show.update_sin(timer.time());
		

		
		tree_offset_X.update_sin(timer.time());
		tree_offset_Y.update_sin(timer.time());

		tree_offset_X.clamp(-1, 1, timer.time());
		tree_offset_Y.clamp(-1, 1, timer.time());

		if (is_active()) {
			float speed = 0.25 * exp(timer.dt);
			if (INTERACTIVE::keys[GLFW_KEY_W]) {
				tree_offset_Y.new_end_position(tree_offset_Y.x() + speed, timer.time());
			}
			if (INTERACTIVE::keys[GLFW_KEY_S]) {
				tree_offset_Y.new_end_position(tree_offset_Y.x() - speed, timer.time());
			}
			if (INTERACTIVE::keys[GLFW_KEY_A]) {
				tree_offset_X.new_end_position(tree_offset_X.x() + speed, timer.time());
			}
			if (INTERACTIVE::keys[GLFW_KEY_D]) {
				tree_offset_X.new_end_position(tree_offset_X.x() - speed, timer.time());
			}
			int powerstation_level = RegionManager::instance_of().get_player().get_building_level_limit(BuildingType::PowerStation);
			int refinery_level = RegionManager::instance_of().get_player().get_building_level_limit(BuildingType::Refinery);
			int steelfactory_level = RegionManager::instance_of().get_player().get_building_level_limit(BuildingType::SteelFactory);
			int civilian_factory_level = RegionManager::instance_of().get_player().get_building_level_limit(BuildingType::CivilFactory);
			int military_factory_level = RegionManager::instance_of().get_player().get_building_level_limit(BuildingType::MilitaryFactory);

			int army_level = RegionManager::instance_of().get_player().get_army_level(0);
			int cm_level = RegionManager::instance_of().get_player().get_army_level(1);
			int mbrm_level = RegionManager::instance_of().get_player().get_army_level(2);
			int icbm_level = RegionManager::instance_of().get_player().get_army_level(3);

			bool research_institution = RegionManager::instance_of().get_player().get_have_research_institution();

			try {

				tree.nodes[0].unlocked = powerstation_level > 0;
				tree.nodes[1].unlocked = refinery_level > 0;
				tree.nodes[2].unlocked = steelfactory_level > 0;
				tree.nodes[3].unlocked = civilian_factory_level > 0;
				tree.nodes[4].unlocked = military_factory_level > 0;

				tree.nodes[5].unlocked = research_institution;

				tree.nodes[6].unlocked = powerstation_level > 1;
				tree.nodes[7].unlocked = refinery_level > 1;
				tree.nodes[8].unlocked = steelfactory_level > 1;
				tree.nodes[9].unlocked = civilian_factory_level > 1;
				tree.nodes[10].unlocked = military_factory_level > 1;

				tree.nodes[11].unlocked = powerstation_level > 2;
				tree.nodes[12].unlocked = refinery_level > 2;
				tree.nodes[13].unlocked = steelfactory_level > 2;
				tree.nodes[14].unlocked = civilian_factory_level > 2;
				tree.nodes[15].unlocked = military_factory_level > 2;

				tree.nodes[16].unlocked = cm_level > 0;
				tree.nodes[17].unlocked = mbrm_level > 0;
				tree.nodes[18].unlocked = icbm_level > 0;

				tree.nodes[19].unlocked = cm_level > 1;
				tree.nodes[20].unlocked = cm_level > 2;

				tree.nodes[21].unlocked = mbrm_level > 1;
				tree.nodes[22].unlocked = mbrm_level > 2;

				tree.nodes[23].unlocked = icbm_level > 1;
				tree.nodes[24].unlocked = icbm_level > 2;

				tree.nodes[25].unlocked = army_level > 0;
				tree.nodes[26].unlocked = army_level > 1;
				tree.nodes[27].unlocked = army_level > 2;
			}
			catch (std::exception e) {
			}
		}


		
	}

	void open(bool open, const Timer& timer) {
		if (open) {
			GAMESOUND::play_popup_sound();
			show.new_end_position(1, timer.time());
			tree_offset_X.new_end_position(0, timer.time());
			tree_offset_Y.new_end_position(0, timer.time());
		}
		else {
			show.new_end_position(0, timer.time());
		}
	}
	bool is_open() {
		return show.x() > 1e-3;
	}
	double getX() {
		return show.x();
	}
} s_tech_tree_gui;

static class ShakeEffect{
	std::vector<SmoothMove> shake_list; // 共给地图震动效果使用
public:
	void clear() {
		shake_list.clear();
	}
	void update(const Timer& timer) {
		for (auto& s : shake_list) {
			s.update_sin(timer.time());
			s.new_end_position(0, timer.time());
		}
		std::vector<SmoothMove> next = {};
		for (auto& s : shake_list) {
			if (s.x() > 1e-3) {
				next.push_back(s);
			}
		}
		shake_list = next;
	}
	void push_shake(const Timer& timer) {
		shake_list.push_back(SmoothMove());
		shake_list.back().set_total_duration(1);
		shake_list.back().set_start_position(1, timer.time());
		shake_list.back().new_end_position(0, timer.time());
	}
	void get_shake_matrix(mat4x4 m) {
		mat4x4 shake_matrix;
		mat4x4_identity(shake_matrix);
		for (auto& s : shake_list) {
			mat4x4 m0;
			Camera cam;
			cam.setPos(sin(s.x() * 50) * 0.0125, sin(s.x() * 25) * 0.0125, sin(s.x() * 12.5) * 0.0125);
			cam.getMat4(m0);

			mat4x4_mul(shake_matrix, shake_matrix, m0);
		}
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				m[i][j] = shake_matrix[i][j];
			}
		}
	}
}s_shake_effect;

static class SelectedGui {
public:
	int grid[2]{}; // 选中的格子
	bool is_selected = false; // 是否选中
	
	SmoothMove shake;

	std::string message;
	
	SelectedGui() {
		shake.set_total_duration(1);
		shake.set_start_position(0, 0);
	}

	void render_gui(ImGuiIO& io) {
		if (shake.x() > 1e-3) {
			ImVec2 shake_offset = ImVec2(sin(shake.x() * 50) * 10, (1 + sin(shake.x() * 25)) * 10);
			ImGui::SetNextWindowBgAlpha(0.5 * shake.x());
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::Begin("Weapon Warning", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoScrollbar);

			ImVec2 text_size = ImGui::CalcTextSize(message.c_str());
			ImGui::SetWindowPos(ImVec2(io.DisplaySize.x / 8 * 3, 0) + shake_offset);
			ImGui::SetWindowSize(ImVec2(io.DisplaySize.x / 4, 50));
			ImGui::SetCursorPosX(io.DisplaySize.x / 8 - text_size.x / 2);			
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, shake.x())); // 前景色
			ImGui::Text(message.c_str());
			ImGui::PopStyleColor();
			ImGui::End();
			ImGui::PopStyleVar();
		}


		if (!is_selected) return;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.5f)); // RGBA 格式
		ImGui::Begin("Selected Grid", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing);

		// 移动到最低端
		Region& region = RegionManager::instance_of().region(grid[0], grid[1]);
		float grid_center_x = region.get_center_position().x / RegionManager::instance_of().map_width() * 2.0 - 1.0;
		float grid_center_z = region.get_center_position().y / RegionManager::instance_of().map_height() * 2.0 - 1.0;
		float grid_center_y = MAP::map_plane_y;

		float screen_pos_x, screen_pos_y;
		get_screen_position(grid_center_x, grid_center_y, grid_center_z, screen_pos_x, screen_pos_y);

		screen_pos_x *= io.DisplaySize.x;
		screen_pos_y *= io.DisplaySize.y;

		//DEBUGOUTPUT(grid_center_x, grid_center_y, grid_center_z, screen_pos_x, screen_pos_y);
		//ImGui::SetWindowPos(ImVec2(io.DisplaySize.x / 8 * 3, io.DisplaySize.y - 200));
		float W = 400 * s_dpi_scale;
		float H = 180 * s_dpi_scale;

		float W_target = 25 * s_dpi_scale;
		float H_target = 25 * s_dpi_scale;

		ImGui::SetWindowPos(ImVec2(screen_pos_x - W / 2, screen_pos_y - H - H_target));

		ImGui::SetWindowSize(ImVec2(W, H));
		switch (s_selected_weapon) {
		case NUCLEAR_MISSILE:
			ImGui::Text(u8"选中: 核导弹");
			// 显示当前选择的武器等级
			ImGui::SameLine();
			ImGui::Text(u8"武器等级: %d", s_nuclear_missile_level + 1);
			break;
		case ARMY:
			ImGui::Text(u8"选中：军队");
			break;
		case SCATTER_BOMB:
			ImGui::Text(u8"选中：散射打击");
			// 滑块颜色
			ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1, 1, 1, 0.5));
			ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(1, 1, 1, 1));
			ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 10.0f);  // 设置滑块大小
			ImGui::SetNextItemWidth(W - 20);  // 设置滑块条宽度
			ImGui::SetCursorPosX(10);
			ImGui::SliderInt("##ScatterBombRange", &s_scatter_bomb_range, 1, MAP::map_info.width() / 2);
			ImGui::PopStyleVar();
			ImGui::PopStyleColor(2);
			break;
		}

		ImGui::Text(u8"区块坐标: %d, %d", grid[0], grid[1]);


		try {
			BuildingType building_type = region.get_building().get_type();
			if (building_type != BuildingType::None)
				ImGui::Text(u8"建筑: %s", BuildingTypeToString(building_type).c_str());
			if (region.get_owner() >= 0) {
				if (region.get_owner() == 0) {
					ImGui::Text(u8"所有者: 玩家");
					if (s_selected_weapon == ARMY) {
						ImGui::Text(u8"军队: %d", region.get_army().get_force());
					}
					if (s_selected_weapon == NUCLEAR_MISSILE) {
						ImGui::Text(u8"导弹数量：%d", region.get_weapons()[s_nuclear_missile_level]);
					}
				}
				else {
					ImGui::Text(u8"所有者: AI");
					if (s_selected_weapon == ARMY) {
						ImGui::Text(u8"军队: %d", region.get_army().get_force());
					}
				}
			}
			ImGui::SameLine();
			ImGui::Text("HP: %.2f", region.get_HP());
		}
		catch (std::exception e) {
			// nothing
		}
		ImGui::End();
		ImGui::PopStyleColor();
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // RGBA 格式

		ImGui::Begin("Selected Grid Target", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing);


		ImGui::SetWindowPos(ImVec2(screen_pos_x - W_target / 2, screen_pos_y - H_target));

		ImGui::SetWindowSize(ImVec2(W_target, H_target));

		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		// 获取窗口位置作为偏移
		ImVec2 pos = ImGui::GetWindowPos();


		// 相对于窗口绘制三角形
		ImVec2 p1 = ImVec2(pos.x, pos.y);
		ImVec2 p2 = ImVec2(pos.x + W_target, pos.y);
		ImVec2 p3 = ImVec2(pos.x + W_target / 2, pos.y + H_target);

		draw_list->AddTriangleFilled(p1, p2, p3, IM_COL32(0, 0, 0, 255 * 0.5));


		ImGui::End();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
	}

	void update(const Timer& timer) {
		shake.update_sin(timer.time());
		shake.new_end_position(0, timer.time());
	}

	void shake_gui(const Timer& timer) {
		shake.set_start_position(1, timer.time());
	}





}s_selected_gui;
static class StatusGui {
private:
	SmoothMove gold_amount{};
	SmoothMove gold_amount_back{};
	SmoothMove electricity_amount{};
	SmoothMove electricity_amount_back{};
	SmoothMove labor_amount{};
	SmoothMove labor_amount_back{};
	SmoothMove oil_amount{};
	SmoothMove oil_amount_back{};
	SmoothMove steel_amount{};
	SmoothMove steel_amount_back{};
	SmoothMove army_amount{};
	SmoothMove army_amount_back{};


	int gold_amount_max = 0;
	int electricity_amount_max = 0;
	int labor_amount_max = 0;
	int oil_amount_max = 0;
	int steel_amount_max = 0;
	int army_amount_max = 0;

	SmoothMove occupation_amount{};
	SmoothMove occupation_amount_back{}; // 已占有资源
	int occupation_amount_max = 0;

	void render_bar(ImGuiIO& io, const SmoothMove& f, const SmoothMove& b, const char* title, GLuint tex, int scale, const ImVec4& fc, const ImVec4& bc, const ImVec4& tc) {
		ImGui::Image(tex, ImVec2(30, 30) * s_dpi_scale);
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, bc); // 前景色
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25f, 0.25f, 0.25f, 0.25f));	   // 背景色
		ImVec2 pos = ImGui::GetCursorPos();
		ImGui::ProgressBar(b.x() / scale, ImVec2(io.DisplaySize.x / 4 - 30, 30) * s_dpi_scale, u8"");
		ImGui::PopStyleColor(2); // 恢复颜色设置
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, fc); // 前景色
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25f, 0.25f, 0.25f, 0.0f));	   // 背景色
		ImGui::SetCursorPos(pos);
		ImGui::ProgressBar(f.x() / scale, ImVec2(io.DisplaySize.x / 4 - 30, 30) * s_dpi_scale, u8"");
		ImGui::PopStyleColor(2); // 恢复颜色设置
		ImVec2 text_size = ImGui::CalcTextSize(title);
		ImGui::SetCursorPos(ImVec2(pos.x + 5 * s_dpi_scale, pos.y + 30 * s_dpi_scale - text_size.y));
		ImGui::PushStyleColor(ImGuiCol_Text, tc); // 前景色
		ImGui::Text(title, (int)round(f.x()));
		ImGui::PopStyleColor(1);
	}

public:
	StatusGui() {
		// 资源条
		{
			gold_amount.set_total_duration(0.125);
			gold_amount_back.set_total_duration(0.5);
			gold_amount_max = 100;
			electricity_amount.set_total_duration(0.125);
			electricity_amount_back.set_total_duration(0.5);
			electricity_amount_max = 100;
			labor_amount.set_total_duration(0.125);
			labor_amount_back.set_total_duration(0.5);
			labor_amount_max = 100;
			oil_amount.set_total_duration(0.125);
			oil_amount_back.set_total_duration(0.5);
			oil_amount_max = 100;
			steel_amount.set_total_duration(0.125);
			steel_amount_back.set_total_duration(0.5);
			steel_amount_max = 100;
			army_amount.set_total_duration(0.125);
			army_amount_back.set_total_duration(0.5);
			army_amount_max = 100;
			occupation_amount.set_total_duration(1);
			occupation_amount_back.set_total_duration(2);
			occupation_amount_max = 100;

		}
	}
	void render_gui(ImGuiIO& io) {
		ImGui::Begin("Status", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse);
		ImGui::SetWindowSize(ImVec2(io.DisplaySize.x / 4, 250) * s_dpi_scale);
		ImGui::SetWindowPos(ImVec2(50 * s_dpi_scale, io.DisplaySize.y - 275 * s_dpi_scale));

		// 资源条
		render_bar(io, gold_amount, gold_amount_back, u8"金钱:%d", TEXTURE::s_image_lightning, gold_amount_max, ImVec4(1,1,0,0.5), ImVec4(1,0,0,0.5), ImVec4(0.15, 0.15, 0.15, 0.5));
		render_bar(io, oil_amount, oil_amount_back, u8"石油:%d", TEXTURE::s_image_lightning, oil_amount_max, ImVec4(0.1, 0.1, 0.1, 1), ImVec4(1, 0.5, 0, 0.5), ImVec4(0.75, 0.75, 0.75, 0.5));
		render_bar(io, steel_amount, steel_amount_back, u8"钢铁:%d", TEXTURE::s_image_lightning, steel_amount_max, ImVec4(1, 1, 1, 1), ImVec4(0.5, 0.5, 0.5, 0.5), ImVec4(0.15, 0.15, 0.15, 0.5));
		render_bar(io, electricity_amount, electricity_amount_back, u8"电力:%d", TEXTURE::s_image_lightning, electricity_amount_max, ImVec4(0, 1, 1, 1), ImVec4(0, 0, 1, 0.5), ImVec4(0.15, 0.15, 0.15, 0.5));
		render_bar(io, labor_amount, labor_amount_back, u8"可用劳动力:%d", TEXTURE::s_image_lightning, labor_amount_max, ImVec4(0.5, 1, 0.5, 0.5), ImVec4(0, 1, 0, 0.5), ImVec4(0.15, 0.15, 0.15, 0.5));
		render_bar(io, army_amount, army_amount_back, u8"军队:%d", TEXTURE::s_image_guard, army_amount_max, ImVec4(0.5, 0.5, 1, 1), ImVec4(0.5, 0.5, 1, 0.5), ImVec4(0.15, 0.15, 0.15, 0.5));
		render_bar(io, occupation_amount, occupation_amount_back, u8"已占有:%d", TEXTURE::s_image_guard, occupation_amount_max, ImVec4(0.6, 0.5, 1, 1), ImVec4(1, 0.25, 0.25, 0.5), ImVec4(0.15, 0.15, 0.15, 0.5));


		ImGui::End();
	}
	int _10_n_max_than(int x) {
		// 计算比x大的最小的10的n次方
		int n = 1;
		while (n < x && n > 0) {
			n *= 10;
		}
		return n;
	}
	void update(const Timer& timer) {
		if (!GAMESTATUS::s_in_game) return;
		gold_amount.update_sin(timer.time());
		gold_amount_back.update_sin(timer.time());
		electricity_amount.update_sin(timer.time());
		electricity_amount_back.update_sin(timer.time());
		labor_amount.update_sin(timer.time());
		labor_amount_back.update_sin(timer.time());
		oil_amount.update_sin(timer.time());
		oil_amount_back.update_sin(timer.time());
		steel_amount.update_sin(timer.time());
		steel_amount_back.update_sin(timer.time());
		army_amount.update_sin(timer.time());
		army_amount_back.update_sin(timer.time());
		occupation_amount.update_sin(timer.time());
		occupation_amount_back.update_sin(timer.time());

		std::vector<double> resources = RegionManager::instance_of().get_player().get_remain_resources();
		std::vector<double> max_resources = RegionManager::instance_of().get_player().get_resources();
		
		double gold = resources[ResourceType::GOLD];
		gold_amount.new_end_position(gold, timer.time());
		gold_amount_back.new_end_position(gold, timer.time());
		gold_amount_max = _10_n_max_than(gold);
		double electricity = resources[ResourceType::ELECTRICITY];
		electricity_amount.new_end_position(electricity, timer.time());
		electricity_amount_back.new_end_position(electricity, timer.time());
		electricity_amount_max = _10_n_max_than(electricity);
		double labor = resources[ResourceType::LABOR];
		labor_amount_max = max_resources[ResourceType::LABOR];
		labor_amount.new_end_position(fmax(0, labor), timer.time());
		labor_amount_back.new_end_position(fmax(0, labor), timer.time());
		double oil = resources[ResourceType::OIL];
		oil_amount.new_end_position(oil, timer.time());
		oil_amount_back.new_end_position(oil, timer.time());
		oil_amount_max = _10_n_max_than(oil);
		double steel = resources[ResourceType::STEEL];
		steel_amount.new_end_position(steel, timer.time());
		steel_amount_back.new_end_position(steel, timer.time());
		steel_amount_max = _10_n_max_than(steel);
		
		int region_count = 0;
		int army_count = 0;
		for (int i{}; i < RegionManager::instance_of().map_width(); i++) {
			for (int j{}; j < RegionManager::instance_of().map_height(); j++) {
				if (RegionManager::instance_of().region(i, j).get_owner() == 0) {
					region_count++;
					army_count += RegionManager::instance_of().region(i, j).get_army().get_force();
				}
			}
		}
		army_amount_max = _10_n_max_than(army_count);
		army_amount.new_end_position(army_count, timer.time());
		army_amount_back.new_end_position(army_count, timer.time());


		occupation_amount_max = RegionManager::instance_of().map_width() * RegionManager::instance_of().map_height();
		double x = region_count;
		occupation_amount.new_end_position(x, timer.time());
		occupation_amount_back.new_end_position(x, timer.time());

	}

}s_status_gui;

static class SubMenuGui
{
	// 弹出建筑选择
	ImVec2 mouse_position;
	int grid[2]{};
	bool open = false;
public:
	void open_gui(bool open, const ImVec2& mouse_position, int grid_x, int grid_y) {
		this->open = open;
		this->mouse_position = mouse_position;
		grid[0] = grid_x;
		grid[1] = grid_y;
	}
	void render_gui(ImGuiIO& io) {
		if (!open) return;
		ImGui::SetNextWindowBgAlpha(0.75);
		ImGui::Begin("Building", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize);
		//ImGui::SetWindowSize(ImVec2(200, 200));
		ImGui::SetWindowPos(mouse_position);
		ImGui::Text(u8"建筑");
		// 列表
		if (ImGui::BeginListBox("##BuildingList")) {
			if (ImGui::Selectable(u8"发电站")) {
				push_input({ Point::to_point(grid),Operator::SetPowerStation });
				open = false;
			}
			if (ImGui::Selectable(u8"炼油厂")) {
				push_input({ Point::to_point(grid),Operator::SetRefinery });
				open = false;
			}
			if (ImGui::Selectable(u8"炼钢厂")) {
				push_input({ Point::to_point(grid),Operator::SetSteelFactory });
				open = false;
			}
			if (ImGui::Selectable(u8"民生工厂")) {
				push_input({ Point::to_point(grid),Operator::SetCivilFactory });
				open = false;
			}
			if (ImGui::Selectable(u8"军事工厂")) {
				push_input({ Point::to_point(grid),Operator::SetMilitaryFactory });
				open = false;
			}
			/*if (ImGui::Selectable(u8"升级建筑")) {
				push_input({ Point::toPoint(grid),Operator::BuildingLevel });
				open = false;
			}*/
			if (ImGui::Selectable(u8"移除建筑")) {
				push_input({ Point::to_point(grid),Operator::RemoveBuilding });
				open = false;
			}
			ImGui::EndListBox();
		}
		// 生产菜单
		ImGui::Text(u8"生产");
		if (ImGui::BeginListBox("##ProductionList")) {
			if (ImGui::Selectable(u8"军队")) {
				push_input({ Point::to_point(grid),Point::to_point(grid),100,Operator::ProductArmy });
				open = false;
			}
			if (ImGui::Selectable(u8"核导弹一级")) {
				push_input({ Point::to_point(grid),Point::to_point(grid),Operator::ProductWeapon0 });
				open = false;
			}
			if (ImGui::Selectable(u8"核导弹二级")) {
				push_input({ Point::to_point(grid),Point::to_point(grid),Operator::ProductWeapon1 });
				open = false;
			}
			if (ImGui::Selectable(u8"核导弹三级")) {
				push_input({ Point::to_point(grid),Point::to_point(grid),Operator::ProductWeapon2 });
				open = false;
			}
			ImGui::EndListBox();
		}
		ImGui::End();
	}
	void update(const Timer& timer) {
		if (s_selected_weapon != NONE) {
			open = false;
		}
		try {
			if (RegionManager::instance_of().region(grid[0], grid[1]).get_owner() != 0) {
				open = false;
			}
		}
		catch (std::exception e) {
			open = false;
		}
		if (grid[0] < 0 || grid[1] < 0) {
			open = false;
		}
	}
	bool is_open() {
		return open;
	}

} s_sub_menu_gui;


static class GameOverGui {
	bool open = false;
public:
	void open_gui(bool open) {
		this->open = open;
	}
	void render_gui(ImGuiIO& io) {
		if (!open) return;
		ImGui::SetNextWindowBgAlpha(0.5);
		ImGui::Begin("Game Over", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
		ImGui::SetWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
		ImGui::SetWindowPos(ImVec2(0, 0));
		ImGui::PushFont(UIFonts::large_font);
		ImVec2 text_size = ImGui::CalcTextSize(u8"游戏结束");
		ImGui::SetCursorPosY(io.DisplaySize.y / 3 - text_size.y);
		ImGui::SetCursorPosX(io.DisplaySize.x / 2 - text_size.x / 2);
		ImGui::Text(u8"游戏结束");
		if (g_game_over) {
			ImVec2 text_size = ImGui::CalcTextSize(u8"AI胜利");
			ImGui::SetCursorPosX(io.DisplaySize.x / 2 - text_size.x / 2);
			ImGui::Text(u8"AI胜利");
		}
		else {
			ImVec2 text_size = ImGui::CalcTextSize(u8"玩家胜利");
			ImGui::SetCursorPosX(io.DisplaySize.x / 2 - text_size.x / 2);
			ImGui::Text(u8"玩家胜利");
		}
		ImGui::PopFont();
		ImGui::End();
	}
	bool is_open() {
		return open;
	}
} s_game_over_gui;

float randfloat() {
	return (float)rand() / RAND_MAX;
}

void render_update_info() {
	RegionManager& rm = RegionManager::instance_of();
	for (int i{}; i < rm.map_width(); i++) {
		for (int j{}; j < rm.map_height(); j++) {
			Region& region_info = rm.region(i, j);
			RegionData region;
			region.cell_center_x = region_info.get_center_position().x - i;
			region.cell_center_y = region_info.get_center_position().y - j;
			region.army_position_x = -1e6;
			region.army_position_y = -1e6;
			region.identity = region_info.get_owner();
			if ((i == RegionManager::instance_of().get_player().get_capital_x() && j == RegionManager::instance_of().get_player().get_capital_y()) && region_info.get_owner() == 0)
				region.region_additional_info = 1;
			else {
				switch (region_info.get_building().get_type()) {
				case BuildingType::PowerStation:
					region.region_additional_info = 2;
					break;
				case BuildingType::Refinery:
					region.region_additional_info = 3;
					break;
				case BuildingType::SteelFactory:
					region.region_additional_info = 4;
					break;
				case BuildingType::CivilFactory:
					region.region_additional_info = 5;
					break;
				case BuildingType::MilitaryFactory:
					region.region_additional_info = 6;
					break;
				default:
					region.region_additional_info = 0;
					break;
				}
			}
			MAP::map_info.set_region(i, j, region);
		}
	}
	MAP::map_info.update();
	
	std::vector<Vertex> vertices;
	auto army = rm.get_moving_army();
	for (auto& a : army) {
		Vertex tmp = { std::get<0>(a.current_pos) / MAP::map_info.width() * 2 - 1, MAP::map_plane_y, std::get<1>(a.current_pos) / MAP::map_info.height() * 2 - 1, 1, 1, 1 };
		if (a.owner_id == 0)
		{
			tmp.r = 0; tmp.g = 1; tmp.b = 0;
		}
		else {
			tmp.r = 1; tmp.g = 0; tmp.b = 0;
		}
		vertices.push_back(tmp);
	}
	auto missile = rm.get_moving_missle();
	MODEL::MISSLE::missle_mesh.clear_matrix();
	float scale = 1.0 / std::fmax(MAP::map_info.width(), MAP::map_info.height());
	float missile_size = 0.1;
	Camera model_camera;
	model_camera.setPos(0, 0, 0);
	for (auto& m : missile) {
		auto [x, z, y] = m.current_pos;
		auto [heading_x, heading_z, heading_y] = m.heading;
		model_camera.setPos(-x * scale * 2 + 1, -y * scale * 2, -z * scale * 2 + 1);
		mat4x4 model_mat;
		model_camera.getMat4(model_mat);
		mat4x4 model_trans;
		vec3 heading_axis = { -heading_x, -heading_y, -heading_z };
		CoordTranslate::mat4x4_align_x_to(model_trans, heading_axis);
		
		mat4x4_scale_aniso(model_trans, model_trans, missile_size * 0.25 * scale, missile_size * 0.25 * scale, missile_size * 0.25 * scale);
		mat4x4 final_model_mat;
		mat4x4_mul(final_model_mat, model_mat, model_trans);
		MODEL::MISSLE::missle_mesh.push_matrix(final_model_mat);

	}
	glDebugMessageCallback((GLDEBUGPROC)0, nullptr);
	MODEL::MISSLE::missle_mesh.update_instance_matrix();
	glDebugMessageCallback((GLDEBUGPROC)debugproc, 0);

	//DEBUGOUTPUT("Army size", army.size());
	PASS_POINT::point_renderer.update(vertices);


}


void load_new_game(const LevelConfig& level_config) {


	DEBUGOUTPUT("Loading new game..");
	DEBUGOUTPUT("Map Width", level_config.map_width);
	DEBUGOUTPUT("Map Height", level_config.map_height);

	MAP::map_info.create(level_config.map_width, level_config.map_height);

	DEBUGOUTPUT("Creating SSBO..");
	for (int i{}; i < level_config.map_width; i++) {
		for (int j{}; j < level_config.map_height; j++) {
			RegionData region;
			region.cell_center_x = 0;
			region.cell_center_y = 0;
			region.army_position_x = -1e6;
			region.army_position_y = -1e6;
			region.identity = 0;
			region.region_additional_info = 0;
			MAP::map_info.set_region(i, j, region);
		}
	}
	MAP::map_info.create_ssbo();
	MAP::map_info.update();
	DEBUGOUTPUT("SSBO Created");


	std::vector<Vertex> vertices;
	PASS_POINT::point_renderer.update(vertices);

	s_tech_tree_gui.init_tech_tree_gui();
	s_shake_effect.clear();


	run_game(level_config.map_width, level_config.map_height);


	GAMESTATUS::s_in_game = true;
	GAMESTATUS::s_enable_control = true;
	DEBUGOUTPUT("Game loaded");	
}

void exit_game() {
	DEBUGOUTPUT("Exiting game..");
	GAMESTATUS::s_in_game = false;
	GAMESTATUS::s_enable_control = false;
	exit_curr_game();
	wait_for_exit();
	MAP::map_info.release();
	DEBUGOUTPUT("Game exited");
}



static class MenuGui {
public:
	enum GuiMode {
		StartGame,
		SelectLevel,
		Pause,
	};
private:
	bool open = false;
	GuiMode m_gui_mode;
	SmoothMove x{};
public:
	MenuGui() {
		x.set_total_duration(0.5);
		x.set_start_position(1, 0);

		// 默认开始游戏
		m_gui_mode = StartGame;
		open = true;
	}
	GuiMode get_gui_mode() {
		return m_gui_mode;
	}
	void open_gui(bool open, const Timer& timer) {
		this->open = open;
		if (open) {
			GAMESOUND::play_popup_sound();
			x.new_end_position(1, timer.time());
		}
		else {
			x.new_end_position(0, timer.time());
		}
	}
	bool is_activitied() {
		return x.x() > 1e-3;
	}
	void render_gui(ImGuiIO& io) {
		if (!is_activitied()) {
			return;
		}
		ImGui::SetNextWindowBgAlpha(0.25 * x.x());
		// 设置边框宽度为0
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		ImGui::Begin("Menu", nullptr,
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar | 
			ImGuiWindowFlags_NoCollapse
		);

		int x_pos = 0;
		ImGui::SetWindowPos(ImVec2(x_pos, 0));
		ImGui::SetWindowSize(io.DisplaySize);
		// 调整字体大小(不使用缩放而是直接使用大号字体）
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25, 0.25, 0.25, 0.25 * x.x()));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35, 0.35, 0.35, 0.5 * x.x()));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15, 0.15, 0.15, 0.5 * x.x()));
		
		// 列表框的
		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 0.55f));        // 选中项背景
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.35f, 0.35f, 0.35f, 0.55f)); // 悬停时背景
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.15f, 0.15f, 0.15f, 0.55f));  // 激活时背景
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.1f, 0.25f));         // 列表框背景色

		

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, x.x()));

		const char* title = "MiniWar";

		switch (m_gui_mode)
		{
		case MenuGui::StartGame:
			ImGui::SetWindowFontScale(64. / 48);
			ImGui::PushFont(UIFonts::large_font);
			break;
		case MenuGui::SelectLevel:
			ImGui::SetWindowFontScale(1);
			ImGui::PushFont(UIFonts::menu_font);
			title = u8"模式";
			break;
		case MenuGui::Pause:
			ImGui::SetWindowFontScale(1);
			ImGui::PushFont(UIFonts::menu_font);
			title = u8"暂停";
			break;
		default:
			break;
		}

		ImVec2 title_size = ImGui::CalcTextSize(title);
		ImGui::SetCursorScreenPos(ImVec2((io.DisplaySize.x - title_size.x) / 2 + x_pos, 50 * s_dpi_scale));
		ImGui::Text(title);
		ImGui::SetWindowFontScale(1);
		ImGui::PopFont();

		switch (m_gui_mode)
		{
		case MenuGui::StartGame:
			ImGui::PushFont(UIFonts::large_font);
			ImGui::SetCursorScreenPos(ImVec2((io.DisplaySize.x - 400 * s_dpi_scale) / 2 + x_pos, 200 * s_dpi_scale));
			if (ImGui::Button(u8"开始游戏", ImVec2(400, 100) * s_dpi_scale)) {
				m_gui_mode = MenuGui::SelectLevel;
				GAMESOUND::play_click_sound();
			}
			ImGui::SetCursorScreenPos(ImVec2((io.DisplaySize.x - 400 * s_dpi_scale) / 2 + x_pos, 360 * s_dpi_scale));
			if (ImGui::Button(u8"退出游戏", ImVec2(400, 100) * s_dpi_scale)) {
				glfwSetWindowShouldClose(glfw_win, true);
			}
			ImGui::PopFont();
			break;
		case MenuGui::SelectLevel:
			ImGui::PushFont(UIFonts::large_font);
			ImGui::SetCursorScreenPos(ImVec2((io.DisplaySize.x - io.DisplaySize.x / 3) / 2 + x_pos, 200 * s_dpi_scale));
			
			if (ImGui::BeginListBox("##LevelSelectionListBox", ImVec2(io.DisplaySize.x / 3, 300 * s_dpi_scale))) {
				for (int n = 0; n < IM_ARRAYSIZE(LEVELDATA::s_levels); n++) {
					const bool is_selected = (LEVELDATA::s_selected_level == n);

					// 计算文本宽度
					float text_width = ImGui::CalcTextSize(LEVELDATA::s_levels[n]).x;
					float item_width = ImGui::GetContentRegionAvail().x;
					float indent = (item_width - text_width) * 0.5f;

					// 添加占位缩进
					ImGui::Dummy(ImVec2(indent, 0));
					ImGui::SameLine();

					// 使用SpanAllColumns确保背景色占满整行
					if (ImGui::Selectable(LEVELDATA::s_levels[n], is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
						GAMESOUND::play_click_sound();
						LEVELDATA::s_selected_level = (LEVELDATA::Level)n;
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
					// 添加下边距
					ImGui::Dummy(ImVec2(0, 5));  // 调整数值可改变行距
				}
				ImGui::EndListBox();
			}
			ImGui::SetCursorScreenPos(ImVec2((io.DisplaySize.x - 400 * s_dpi_scale) / 2 + x_pos, 525 * s_dpi_scale));

			if (ImGui::Button(u8"载入游戏", ImVec2(400, 100) * s_dpi_scale)) {
				GAMESOUND::play_click_sound();
				m_gui_mode = MenuGui::Pause;
				switch (LEVELDATA::s_selected_level)
				{
				case LEVELDATA::EASY:
					load_new_game({ 16, 16 });
					break;
				case LEVELDATA::NORMAL:
					load_new_game({ 32, 32 });
					break;
				case LEVELDATA::HARD:
					load_new_game({ 64, 64 });
					break;
				case LEVELDATA::CRAZY:
					load_new_game({ 128, 128 });
					break;
				default:
					break;
				}
				open_gui(false, RENDERER::timer);
			}
			ImGui::SetCursorScreenPos(ImVec2((io.DisplaySize.x - 400 * s_dpi_scale) / 2 + x_pos, 650 * s_dpi_scale));

			if (ImGui::Button(u8"返回主界面", ImVec2(400, 100) * s_dpi_scale)) {
				GAMESOUND::play_click_sound();
				m_gui_mode = MenuGui::StartGame;
			}
			ImGui::PopFont();
			break;
		case MenuGui::Pause:
			ImGui::PushFont(UIFonts::large_font);
			ImGui::SetCursorScreenPos(ImVec2((io.DisplaySize.x - 400 * s_dpi_scale) / 2 + x_pos, 200 * s_dpi_scale));
			if (ImGui::Button(u8"继续游戏", ImVec2(400, 100) * s_dpi_scale)) {
				GAMESOUND::play_click_sound();
				open_gui(false, RENDERER::timer);
			}
			ImGui::SetCursorScreenPos(ImVec2((io.DisplaySize.x - 400 * s_dpi_scale) / 2 + x_pos, 360 * s_dpi_scale));
			if (ImGui::Button(u8"返回主界面", ImVec2(400, 100) * s_dpi_scale)) {
				GAMESOUND::play_click_sound();
				exit_game();
				m_gui_mode = MenuGui::StartGame;
			}
			ImGui::SetCursorScreenPos(ImVec2((io.DisplaySize.x - 400 * s_dpi_scale) / 2 + x_pos, 520 * s_dpi_scale));
			if (ImGui::Button(u8"退出游戏", ImVec2(400, 100) * s_dpi_scale)) {
				GAMESOUND::play_click_sound();
				exit_game();
				glfwSetWindowShouldClose(glfw_win, true);
			}
			ImGui::PopFont();
			break;
		default:
			break;
		}

		ImGui::PopStyleColor(8);
		ImGui::End();

		// 恢复样式
		ImGui::PopStyleVar();

	}
	void update(const Timer& timer) {
		x.update_sin(timer.time());
	}
	bool is_open() const{
		return open;
	}
	void set_move_time(double time) {
		x.set_total_duration(time);
	}
	double getX() {
		return x.x();
	}
} s_menu_gui;


//float s_test_float;


void render_imgui(ImGuiIO& io) {
	if (!s_menu_gui.is_activitied()) {
		int glfw_width, glfw_height;
		glfwGetWindowSize(glfw_win, &glfw_width, &glfw_height);


		ImGui::SetNextWindowBgAlpha(0.25);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::Begin("SelectedWeapon", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::SetWindowSize(ImVec2(120, 120) * s_dpi_scale);
		ImGui::SetWindowPos(ImVec2(io.DisplaySize.x - 130 * s_dpi_scale, io.DisplaySize.y - 130 * s_dpi_scale));
		ImGui::SetCursorPos(ImVec2(10, 10) * s_dpi_scale);
		switch (s_selected_weapon)
		{
		case NUCLEAR_MISSILE:
			ImGui::Image(TEXTURE::s_image_radioactive, ImVec2(100, 100) * s_dpi_scale);
			break;
		case ARMY:
			ImGui::Image(TEXTURE::s_image_attack_target, ImVec2(100, 100) * s_dpi_scale);
			break;
		case SCATTER_BOMB:
			ImGui::Image(TEXTURE::s_image_scatter, ImVec2(100, 100) * s_dpi_scale);
			break;
		default:
			break;
		}

		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && GAMESTATUS::s_enable_control) {
			KeyRelease(GLFW_KEY_SPACE); // 切换武器
		}

		ImGui::End();
		ImGui::PopStyleVar();
		s_selected_gui.render_gui(io);
		s_status_gui.render_gui(io);
		s_sub_menu_gui.render_gui(io);
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
			s_sub_menu_gui.open_gui(false, ImVec2(io.MousePos.x, io.MousePos.y), INTERACTIVE::s_current_selected_grid[0], INTERACTIVE::s_current_selected_grid[1]);
			if (GAMESTATUS::s_enable_control) {
				// 显示格子信息
				if (INTERACTIVE::s_is_selected && INTERACTIVE::s_current_selected_grid[0] == s_selected_gui.grid[0] && INTERACTIVE::s_current_selected_grid[1] == s_selected_gui.grid[1]) {
					s_selected_gui.is_selected = !s_selected_gui.is_selected;
					GAMESOUND::play_click_sound();
				}
				else {
					s_selected_gui.is_selected = INTERACTIVE::s_is_selected;
					s_selected_gui.grid[0] = INTERACTIVE::s_current_selected_grid[0];
					s_selected_gui.grid[1] = INTERACTIVE::s_current_selected_grid[1];
					GAMESOUND::play_click_sound();
				}
			}
		}
		// 检查窗口是否被点击
		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && GAMESTATUS::s_enable_control) {
			s_selected_gui.is_selected = INTERACTIVE::s_is_selected;
			s_selected_gui.grid[0] = INTERACTIVE::s_current_selected_grid[0];
			s_selected_gui.grid[1] = INTERACTIVE::s_current_selected_grid[1];
			s_sub_menu_gui.open_gui(INTERACTIVE::s_is_selected, ImVec2(io.MousePos.x, io.MousePos.y), INTERACTIVE::s_current_selected_grid[0], INTERACTIVE::s_current_selected_grid[1]);
			GAMESOUND::play_click_sound();
		}

		// 检查鼠标是否移动
		if (ImGui::IsWindowHovered() && GAMESTATUS::s_enable_control) {
			ImVec2 mouse_pos = ImGui::GetMousePos();
			INTERACTIVE::s_mouse_position[0] = mouse_pos.x; INTERACTIVE::s_mouse_position[1] = mouse_pos.y;
		}

		//ImGui::SetCursorPos(ImVec2(0, 0));
		//ImGui::SliderFloat("Test", &s_test_float, 0, 5);


		ImGui::End();


		s_tech_tree_gui.render_gui(io);
		s_game_over_gui.render_gui(io);
		
	}
	s_menu_gui.render_gui(io);
}

void render_main_game_pass() {
	FBO::g_main_game_pass_fbo.bind_frameBuffer();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	// 深度小于等于
	glDepthFunc(GL_LEQUAL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	int W, H;
	float ratio;
	mat4x4 m, p, mvp;

	glfwGetFramebufferSize(glfw_win, &W, &H);

	ratio =  W / (float)H;
	int mvp_location = glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "MVP");

	mat4x4_identity(m);
	mat4x4_ortho(p, -1, 1, -1.f, 1.f, 1.f, -1.f);
	mat4x4_mul(mvp, p, m);


	glUseProgram(PASS_MAP_RENDER::s_map_renderer_program);
	glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);

	
	MAP::map_info.bind(0);

	// g_fov
	glUniform1f(glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "g_fov"), -2.0);
	//g_frame_width, g_frame_height
	glUniform1f(glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "g_frame_width"), W);
	glUniform1f(glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "g_frame_height"), H);
	glUniform1f(glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "g_time"), (float)RENDERER::timer.time());
	glUniform2i(glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "g_map_size"), MAP::map_info.width(), MAP::map_info.height());


	Camera model_camera;
	model_camera.setPos(0, 0.5, 0);
	model_camera.setRot(0, -MAP::map_rotation.x(), 0);
	mat4x4 model_mat;
	model_camera.getMat4(model_mat);
	mat4x4_scale_aniso(model_mat, model_mat, 0.25, 0.25, 0.25);

	glUniformMatrix4fv(glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "g_model_trans_mat"), 1, GL_FALSE, (const GLfloat*)model_mat);

	mat4x4 model_mat_inv_rot;
	model_camera.setPos(0, 0.5, 0);
	model_camera.setRot(0, MAP::map_rotation.x(), 0);
	model_camera.getMat4(model_mat_inv_rot);
	mat4x4_scale_aniso(model_mat_inv_rot, model_mat_inv_rot, 0.25, 0.25, 0.25);

	glUniformMatrix4fv(glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "g_model_trans_mat_inv"), 1, GL_FALSE, (const GLfloat*)model_mat_inv_rot);

	mat4x4 g_trans_mat;
	MAP::camera.getCamera().getMat4(g_trans_mat);
	mat4x4 g_scale_mat;
	MAP::scale_map_camera.getCamera().getMat4(g_scale_mat);
	mat4x4_mul(g_trans_mat, g_trans_mat, g_scale_mat);

	mat4x4 shake_camera;
	s_shake_effect.get_shake_matrix(shake_camera);
	mat4x4_mul(g_trans_mat, g_trans_mat, shake_camera);


	int g_trans_mat_location = glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "g_trans_mat");
	glUniformMatrix4fv(g_trans_mat_location, 1, GL_FALSE, (const GLfloat*)g_trans_mat);




	auto [selected, gridX, gridY] = RegionSelector(-2.0, W, H, g_trans_mat, model_mat, MAP::map_info.width(), MAP::map_info.height(), MAP::map_info.regions())(INTERACTIVE::s_mouse_position[0], INTERACTIVE::s_mouse_position[1]);

	INTERACTIVE::s_current_selected_grid[0] = gridX;
	INTERACTIVE::s_current_selected_grid[1] = gridY;
	INTERACTIVE::s_is_selected = selected;

	if (s_selected_gui.is_selected) {
		glUniform2i(glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "g_selected"), s_selected_gui.grid[0], s_selected_gui.grid[1]);
	}
	else {
		glUniform2i(glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "g_selected"), -1, -1);
	}
	if (selected) {
		glUniform2i(glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "g_mouse_selected"), gridX, gridY);
	}
	else {
		glUniform2i(glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "g_mouse_selected"), -1, -1);
	}

	bool s_if_selected = s_selected_gui.is_selected;

	// 核辐射标识
	glUniform4f(glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "g_radioactive_selected"), gridX, gridY, RegionManager::instance_of().get_weapon(s_nuclear_missile_level).get_damage_range(RegionManager::instance_of().get_player().get_army_level(s_nuclear_missile_level + 1)), s_selected_weapon == NUCLEAR_MISSILE && s_if_selected);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TEXTURE::s_image_radioactive);
	glUniform1i(glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "g_tex_radioactive"), 0);

	// 攻击目标标识
	glUniform3f(glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "g_attack_target"), gridX, gridY, s_selected_weapon == ARMY && s_if_selected);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, TEXTURE::s_image_attack_target);
	glUniform1i(glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "g_tex_attack_target"), 1);

	// 散弹炸弹标识
	glUniform4f(glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "g_scatter_target"), gridX, gridY, s_scatter_bomb_range, s_selected_weapon == SCATTER_BOMB && s_if_selected);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, TEXTURE::s_image_scatter);
	glUniform1i(glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "g_tex_scatter_target"), 2);

	// 禁止攻击标识
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, TEXTURE::s_image_forbid);
	glUniform1i(glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "g_tex_forbid"), 3);

	// 建筑标识
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, TEXTURE::s_image_building_icon);
	glUniform1i(glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "g_tex_building_icon"), 4);


	float attack_range = 0;
	switch (s_selected_weapon)
	{
	case NONE:
		break;
	case NUCLEAR_MISSILE:
		attack_range = std::get<1>(RegionManager::instance_of().get_weapon(s_nuclear_missile_level).get_attack_range()) * fmax(RegionManager::instance_of().map_width(), RegionManager::instance_of().map_height());
		break;
	case ARMY:
		attack_range = 1e6;
		break;
	case SCATTER_BOMB:
		attack_range = 40;
		break;
	default:
		break;
	}

	glUniform1f(glGetUniformLocation(PASS_MAP_RENDER::s_map_renderer_program, "g_valid_attack_range"), attack_range);
	// 渲染！
	MAP::map_mesh.render(PASS_MAP_RENDER::s_map_renderer_program, "vPos", nullptr, nullptr, nullptr);

	glUseProgram(0);
	MAP::map_info.unbind();

	FBO::g_main_game_pass_fbo.unbind_frameBuffer();
}

void prepare_render() {
	GAMESOUND::check_if_need_play_next();


	MAP::scale_map_camera.update(RENDERER::timer.time());
	MAP::scale_map_camera.clampZ(-80, 0,RENDERER::timer.time());

	MAP::camera.update(RENDERER::timer.time());
	MAP::camera.clampX(-5, 5, RENDERER::timer.time());
	MAP::camera.clampZ(-6, 4, RENDERER::timer.time());

	MAP::map_rotation.update_sin(RENDERER::timer.time());

	s_menu_gui.update(RENDERER::timer);
	s_status_gui.update(RENDERER::timer);
	s_tech_tree_gui.update(RENDERER::timer);
	s_selected_gui.update(RENDERER::timer);
	s_sub_menu_gui.update(RENDERER::timer);

	s_shake_effect.update(RENDERER::timer);

	GAMESTATUS::s_enable_control = !s_menu_gui.is_activitied() && !s_tech_tree_gui.is_open() && !s_sub_menu_gui.is_open() && !s_game_over_gui.is_open();

	if (GAMESTATUS::s_in_game) {
		GAMESOUND::set_background_volume(0.5);
		render_update_info();
	}
	else {
		GAMESOUND::set_background_volume(1);
	}

	std::vector<std::string> errs = get_error_messages();
	if (errs.size() > 0) {
		s_selected_gui.message = errs[0];
		s_selected_gui.shake_gui(RENDERER::timer);
		GAMESOUND::play_error_sound();
	}

	std::vector<GameEffect> effects = get_game_effects();
	if (effects.size() > 0) {

		for(auto& e: effects){
			switch (e)
			{
			case GameEffect::GAME_EFFECT_PLAY_NUCLEAR_EXPLOSION:
				s_shake_effect.push_shake(RENDERER::timer);
				GAMESOUND::play_bomb_explosion_sound();
				break;
			case GameEffect::GAME_EFFECT_PLAY_NUCLEAR_WARNING:
				GAMESOUND::play_nuclear_launch_sound();
				break;
			default:
				break;
			}
		}
	}

	s_game_over_gui.open_gui(g_game_stop);
}

// 添加新的结构体定义
struct CommonTransform {
	int window_width;
	int window_height;
	mat4x4 mvp;
	mat4x4 model_mat_inv_rot;
	mat4x4 trans_mat;
};

void calculate_common_transform(CommonTransform& transform, float fov = 2.0f) {
	// 窗口尺寸
	glfwGetFramebufferSize(glfw_win, &transform.window_width, &transform.window_height);

	// 计算MVP矩阵
	CoordTranslate::project(transform.mvp, transform.window_width, transform.window_height, fov);

	// 计算模型变换矩阵
	Camera model_camera;
	model_camera.setPos(0, 0.5, 0);
	model_camera.setRot(0, MAP::map_rotation.x(), 0);
	model_camera.getMat4(transform.model_mat_inv_rot);
	mat4x4_scale_aniso(transform.model_mat_inv_rot, transform.model_mat_inv_rot, 0.25, 0.25, 0.25);

	// 计算相机变换矩阵
	mat4x4 g_scale_mat;
	MAP::camera.getCamera().getMat4(transform.trans_mat);
	MAP::scale_map_camera.getCamera().getMat4(g_scale_mat);
	mat4x4_mul(transform.trans_mat, transform.trans_mat, g_scale_mat);

	mat4x4 shake_camera;
	s_shake_effect.get_shake_matrix(shake_camera);
	mat4x4_mul(transform.trans_mat, transform.trans_mat, shake_camera);
}

void render_points() {
	CommonTransform transform;
	calculate_common_transform(transform);

	
	FBO::g_main_game_pass_fbo.bind_frameBuffer();

	int mvp_location = glGetUniformLocation(PASS_POINT::s_points_program, "MVP");
	glUseProgram(PASS_POINT::s_points_program);
	glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)transform.mvp);

	glUniform1f(glGetUniformLocation(PASS_POINT::s_points_program, "g_fov"), -2.0);
	glUniform1f(glGetUniformLocation(PASS_POINT::s_points_program, "g_frame_width"), transform.window_width);
	glUniform1f(glGetUniformLocation(PASS_POINT::s_points_program, "g_frame_height"), transform.window_height);
	glUniform1f(glGetUniformLocation(PASS_POINT::s_points_program, "g_time"), (float)RENDERER::timer.time());
	glUniform2i(glGetUniformLocation(PASS_POINT::s_points_program, "g_map_size"), MAP::map_info.width(), MAP::map_info.height());

	glUniformMatrix4fv(glGetUniformLocation(PASS_POINT::s_points_program, "g_model_trans_mat_inv"), 1, GL_FALSE, (const GLfloat*)transform.model_mat_inv_rot);
	glUniformMatrix4fv(glGetUniformLocation(PASS_POINT::s_points_program, "g_trans_mat"), 1, GL_FALSE, (const GLfloat*)transform.trans_mat);

	PASS_POINT::point_renderer.render(PASS_POINT::s_points_program);
	glUseProgram(0);
	FBO::g_main_game_pass_fbo.unbind_frameBuffer();
}

void get_screen_position(float x, float y, float z, float& screen_x, float& screen_y) {
	CommonTransform transform;
	calculate_common_transform(transform);

	vec3 world_pos = { x,y,z };
	vec2 screen_pos = { 0,0 };
	CoordTranslate::world_to_screen(screen_pos, world_pos, transform.mvp, transform.model_mat_inv_rot, transform.trans_mat);
	screen_x = screen_pos[0];
	screen_y = screen_pos[1];
}

void render_missile() {
	CommonTransform transform;
	calculate_common_transform(transform);

	mat4x4 model_trans_mat;
	mat4x4_identity(model_trans_mat);
	mat4x4_translate_in_place(model_trans_mat, 0, MAP::map_plane_y, 0);
	//float scale = 1.0 / std::fmax(MAP::map_info.width(), MAP::map_info.height());
	//mat4x4_scale_aniso(model_trans_mat, model_trans_mat, scale, scale, scale);
	mat4x4 inv_mat;
	mat4x4_invert(inv_mat, model_trans_mat);
	mat4x4 tmp_model_mat_inv_rot;
	mat4x4_mul(tmp_model_mat_inv_rot, inv_mat, transform.model_mat_inv_rot);

	FBO::g_main_game_pass_fbo.bind_frameBuffer();

	int mvp_location = glGetUniformLocation(PASS_POINT::s_points_program, "MVP");
	glUseProgram(PASS_NORMAL::s_normal_gl_program);
	glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)transform.mvp);

	glUniform1f(glGetUniformLocation(PASS_POINT::s_points_program, "g_fov"), -2.0);
	glUniform1f(glGetUniformLocation(PASS_POINT::s_points_program, "g_frame_width"), transform.window_width);
	glUniform1f(glGetUniformLocation(PASS_POINT::s_points_program, "g_frame_height"), transform.window_height);
	glUniform1f(glGetUniformLocation(PASS_POINT::s_points_program, "g_time"), (float)RENDERER::timer.time());
	glUniform2i(glGetUniformLocation(PASS_POINT::s_points_program, "g_map_size"), MAP::map_info.width(), MAP::map_info.height());

	glUniformMatrix4fv(glGetUniformLocation(PASS_POINT::s_points_program, "g_model_trans_mat_inv"), 1, GL_FALSE, (const GLfloat*)tmp_model_mat_inv_rot);
	glUniformMatrix4fv(glGetUniformLocation(PASS_POINT::s_points_program, "g_trans_mat"), 1, GL_FALSE, (const GLfloat*)transform.trans_mat);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, MODEL::MISSLE::missle_texture);


	MODEL::MISSLE::missle_mesh.enable_instance = true;
	MODEL::MISSLE::missle_mesh.render(PASS_NORMAL::s_normal_gl_program, "vPos", "vColor", "vUV", "vNormal", "vModelMat");

	glUseProgram(0);
	FBO::g_main_game_pass_fbo.unbind_frameBuffer();
}


void render_gaussian_blur() {
	
	{
		FBO::g_gaussian_blur_pass_fbo.bind_frameBuffer();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		int W, H;
		float ratio;
		mat4x4 m, p, mvp;

		glfwGetFramebufferSize(glfw_win, &W, &H);

		ratio = W / (float)H;
		int mvp_location = glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "MVP");

		mat4x4_identity(m);
		mat4x4_ortho(p, -1, 1, -1.f, 1.f, 1.f, -1.f);
		mat4x4_mul(mvp, p, m);

		glUseProgram(PASS_BLOOM::s_gaussian_blur_program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);

		glActiveTexture(GL_TEXTURE0);
		FBO::g_main_game_pass_fbo.bind_texture();
		glGenerateMipmap(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE1);
		FBO::g_gaussian_blur_vertical_pass_fbo.bind_texture();
		glGenerateMipmap(GL_TEXTURE_2D);
		glUniform1i(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_from_origin"), true);
		glUniform1i(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_blur_radius"), 1);
		glUniform1f(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_step"), 0.125);
		glUniform1f(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_w_div_h"), ratio);
		glUniform1i(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_gaussian"), false);
		glUniform1i(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_vertical"), false);

		MAP::s_mesh.render(PASS_BLOOM::s_gaussian_blur_program, "vPos", nullptr, "vUV", nullptr);
		
		glUseProgram(0);
		FBO::g_gaussian_blur_pass_fbo.unbind_frameBuffer();
	}
	{
		FBO::g_gaussian_blur_vertical_pass_fbo.bind_frameBuffer();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		int W, H;
		float ratio;
		mat4x4 m, p, mvp;

		glfwGetFramebufferSize(glfw_win, &W, &H);

		ratio = W / (float)H;
		int mvp_location = glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "MVP");

		mat4x4_identity(m);
		mat4x4_ortho(p, -1, 1, -1.f, 1.f, 1.f, -1.f);
		mat4x4_mul(mvp, p, m);


		glUseProgram(PASS_BLOOM::s_gaussian_blur_program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);

		glActiveTexture(GL_TEXTURE0);
		FBO::g_main_game_pass_fbo.bind_texture();
		glActiveTexture(GL_TEXTURE1);
		FBO::g_gaussian_blur_pass_fbo.bind_texture();
		glGenerateMipmap(GL_TEXTURE_2D);
		glUniform1i(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_from_origin"), false);
		glUniform1i(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_blur_radius"), 1);
		glUniform1f(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_step"), 0.125);
		glUniform1f(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_w_div_h"), ratio);
		glUniform1i(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_gaussian"), false);
		glUniform1i(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_vertical"), false);

		MAP::s_mesh.render(PASS_BLOOM::s_gaussian_blur_program, "vPos", nullptr, "vUV", nullptr);

		glUseProgram(0);
		FBO::g_gaussian_blur_vertical_pass_fbo.unbind_frameBuffer();
	}


	{
		FBO::g_gaussian_blur_pass_fbo.bind_frameBuffer();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		int W, H;
		float ratio;
		mat4x4 m, p, mvp;

		glfwGetFramebufferSize(glfw_win, &W, &H);

		ratio = W / (float)H;
		int mvp_location = glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "MVP");

		mat4x4_identity(m);
		mat4x4_ortho(p, -1, 1, -1.f, 1.f, 1.f, -1.f);
		mat4x4_mul(mvp, p, m);


		glUseProgram(PASS_BLOOM::s_gaussian_blur_program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);

		glActiveTexture(GL_TEXTURE0);
		FBO::g_main_game_pass_fbo.bind_texture();
		glActiveTexture(GL_TEXTURE1);
		FBO::g_gaussian_blur_vertical_pass_fbo.bind_texture();
		glGenerateMipmap(GL_TEXTURE_2D);
		glUniform1i(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_from_origin"), false);
		glUniform1i(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_blur_radius"), 8);
		glUniform1f(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_step"), 0.025);
		glUniform1f(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_w_div_h"), ratio);
		glUniform1i(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_gaussian"), true);
		glUniform1i(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_vertical"), true);

		MAP::s_mesh.render(PASS_BLOOM::s_gaussian_blur_program, "vPos", nullptr, "vUV", nullptr);

		glUseProgram(0);
		FBO::g_gaussian_blur_pass_fbo.unbind_frameBuffer();
	}
	{
		FBO::g_gaussian_blur_vertical_pass_fbo.bind_frameBuffer();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		int W, H;
		float ratio;
		mat4x4 m, p, mvp;

		glfwGetFramebufferSize(glfw_win, &W, &H);

		ratio = W / (float)H;
		int mvp_location = glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "MVP");

		mat4x4_identity(m);
		mat4x4_ortho(p, -1, 1, -1.f, 1.f, 1.f, -1.f);
		mat4x4_mul(mvp, p, m);


		glUseProgram(PASS_BLOOM::s_gaussian_blur_program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);

		glActiveTexture(GL_TEXTURE0);
		FBO::g_main_game_pass_fbo.bind_texture();
		glActiveTexture(GL_TEXTURE1);
		FBO::g_gaussian_blur_pass_fbo.bind_texture();
		glGenerateMipmap(GL_TEXTURE_2D);
		glUniform1i(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_from_origin"), false);
		glUniform1i(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_blur_radius"), 8);
		glUniform1f(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_step"), 0.025);
		glUniform1f(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_w_div_h"), ratio);
		glUniform1i(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_gaussian"), true);
		glUniform1i(glGetUniformLocation(PASS_BLOOM::s_gaussian_blur_program, "g_vertical"), false);

		MAP::s_mesh.render(PASS_BLOOM::s_gaussian_blur_program, "vPos", nullptr, "vUV", nullptr);
		glUseProgram(0);
		FBO::g_gaussian_blur_vertical_pass_fbo.unbind_frameBuffer();
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
	int mvp_location = glGetUniformLocation(PASS_DIRECT_TEX::s_direct_tex_program, "MVP");

	mat4x4_identity(m);
	mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
	mat4x4_mul(mvp, p, m);

	glActiveTexture(GL_TEXTURE0);

	if (s_menu_gui.is_activitied()) {
		FBO::g_flame_render_pass.render(FBO::g_final_mix_pass_fbo.get_texture(), RENDERER::timer, MAP::s_mesh, s_menu_gui.getX());
		FBO::g_flame_render_pass.get_fbo().bind_texture();
	}
	else
		FBO::g_final_mix_pass_fbo.bind_texture();

	glUseProgram(PASS_DIRECT_TEX::s_direct_tex_program);
	glUniform1i(glGetUniformLocation(PASS_DIRECT_TEX::s_direct_tex_program, "g_pass"), 0);
	glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);

	MAP::s_mesh.render(PASS_DIRECT_TEX::s_direct_tex_program, "vPos", nullptr, "vUV", nullptr);

	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);
}

void render() {
	// HDR
	//glEnable(GL_FRAMEBUFFER_SRGB);
	//glEnable(GL_COLOR_LOGIC_OP);
	glDisable(GL_BLEND);
	if (!s_menu_gui.is_activitied() && GAMESTATUS::s_in_game) {
		render_main_game_pass();
		render_points();
		render_missile();

		// 检查是否为arm平台
//#if defined(__aarch64__) || defined(__arm__)
//		FBO::g_main_game_pass_fbo.bind_texture();
//		glGenerateMipmap(GL_TEXTURE_2D);
//#else
		render_gaussian_blur();
//#endif
	}
	glUseProgram(0);
	FBO::g_final_mix_pass_fbo.bind_frameBuffer();
	int W, H;
	float ratio;
	mat4x4 m, p, mvp;
	glfwGetFramebufferSize(glfw_win,&W,&H);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	ratio = 1;
	int mvp_location = glGetUniformLocation(PASS_MAIN::s_main_game_pass_program, "MVP");

	mat4x4_identity(m);
	mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
	mat4x4_mul(mvp, p, m);

	glActiveTexture(GL_TEXTURE0);
	FBO::g_main_game_pass_fbo.bind_texture();
	glActiveTexture(GL_TEXTURE1);
	FBO::g_gaussian_blur_vertical_pass_fbo.bind_texture();
	glGenerateMipmap(GL_TEXTURE_2D);
	glUseProgram(PASS_MAIN::s_main_game_pass_program);
	glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);

	// 菜单模糊/泛光
	if (s_menu_gui.is_activitied())
//#if defined(__aarch64__) || defined(__arm__)
//		glUniform1f(glGetUniformLocation(PASS_MAIN::s_main_game_pass_program, "g_blur"), 0.0);
//#else
		glUniform1f(glGetUniformLocation(PASS_MAIN::s_main_game_pass_program, "g_blur"), 0.05 + 0.95 * s_menu_gui.getX());
//#endif
	else if (s_tech_tree_gui.is_active()) {
//#if defined(__aarch64__) || defined(__arm__)
//		glUniform1f(glGetUniformLocation(PASS_MAIN::s_main_game_pass_program, "g_blur"), 0.0);
//#else
		glUniform1f(glGetUniformLocation(PASS_MAIN::s_main_game_pass_program, "g_blur"), 0.05 + 0.95 * s_tech_tree_gui.getX());
//#endif
	}
	else {
//#if defined(__aarch64__) || defined(__arm__)
//		glUniform1f(glGetUniformLocation(PASS_MAIN::s_main_game_pass_program, "g_blur"), 0.0);
//#else
		glUniform1f(glGetUniformLocation(PASS_MAIN::s_main_game_pass_program, "g_blur"), 0.05);
//#endif
	}
	MAP::s_mesh.render(PASS_MAIN::s_main_game_pass_program, "vPos", nullptr, "vUV", nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	
	FBO::g_final_mix_pass_fbo.unbind_frameBuffer();

	render_final();

	//glDisable(GL_COLOR_LOGIC_OP);
	//glDisable(GL_FRAMEBUFFER_SRGB);
	//glfwSwapBuffers(glfw_win);

}

void reset_camera() {
	MAP::camera.move_to(0, 0, -1.75, RENDERER::timer.time());
	MAP::camera.rotate_to(0, 0, -1.2, RENDERER::timer.time());
	MAP::scale_map_camera.move_to(0, 0, -8, RENDERER::timer.time());
	MAP::camera.rotate_to(0, 0, -1.2 / (1 + 2 * exp(-0.05 * -8 * -8)), RENDERER::timer.time());
	MAP::map_rotation.new_end_position(-(exp(-0.01 * pow(-8 * -8, 2))), RENDERER::timer.time());
}

void KeyProcess() {
	if (s_menu_gui.is_open() || !GAMESTATUS::s_enable_control) {
		return;
	}

	float dx = 0, dz = 0;

	float speed = - 0.5 * (MAP::scale_map_camera.getZ() - 1);

	if (INTERACTIVE::keys[GLFW_KEY_W]) {
		dz += speed;
	}
	if (INTERACTIVE::keys[GLFW_KEY_S]) {
		dz += -speed;
	}
	if (INTERACTIVE::keys[GLFW_KEY_A]) {
		dx += speed;
	}
	if (INTERACTIVE::keys[GLFW_KEY_D]) {
		dx += -speed;
	}

	if (dx != 0 || dz != 0) {
		dx *= exp(RENDERER::timer.dt);
		dz *= exp(RENDERER::timer.dt);
		MAP::camera.move(dx, 0, dz, RENDERER::timer.time());

	}
	if (INTERACTIVE::keys[GLFW_KEY_R]) {
		// 重置摄像机
		reset_camera();
	}
}

void KeyRelease(int key) {
	if(key == GLFW_KEY_ESCAPE)
		if (s_menu_gui.get_gui_mode() == MenuGui::GuiMode::Pause) {
			s_menu_gui.open_gui(!s_menu_gui.is_open(), RENDERER::timer);
		}
	
	if (s_menu_gui.is_open()) {
		return;
	}
	switch (key)
	{
	case GLFW_KEY_SPACE:
		if (GAMESTATUS::s_enable_control) {
			GAMESOUND::play_click_sound();
			s_selected_weapon = (SelectedWeapon)((s_selected_weapon + 1) % 3);		
		}
		break;
	case GLFW_KEY_T:
		s_tech_tree_gui.open(!s_tech_tree_gui.is_open(), RENDERER::timer);
		break;
	case GLFW_KEY_E: // 发动攻击
		if (GAMESTATUS::s_enable_control) {
			if (s_selected_gui.is_selected) {
				Point start_point = Point::to_point(s_selected_gui.grid);
				Point end_point = Point::to_point(INTERACTIVE::s_current_selected_grid);
				try {
					Region& from_region = RegionManager::instance_of().region(start_point.x, start_point.y);
					Region& to_region = RegionManager::instance_of().region(end_point.x, end_point.y);
					std::string result = "";
					switch (s_selected_weapon)
					{
					case NONE:
						//push_input({ start_point, end_point, Operator::ArmyMove });
						break;
					case NUCLEAR_MISSILE:
						if (from_region.get_owner() != 0) {
							s_selected_gui.message = u8"无效区块！";
							s_selected_gui.shake_gui(RENDERER::timer);
							break;
						}

						switch (s_nuclear_missile_level)
						{
						case 0:
							result = push_input_wait_for_result({ start_point, end_point, Operator::Weapon0Attack });
							break;
						case 1:
							result = push_input_wait_for_result({ start_point, end_point, Operator::Weapon1Attack });
							break;
						case 2:
							result = push_input_wait_for_result({ start_point, end_point, Operator::Weapon2Attack });
							break;
						default:
							break;
						}
						if (result == "Success") {
							DEBUGOUTPUT("Nuclear Missile");
							GAMESOUND::play_nuclear_launch_sound();
							s_selected_gui.message = u8"核导弹已发射";
							s_selected_gui.shake_gui(RENDERER::timer);
							s_shake_effect.push_shake(RENDERER::timer);
						}
						break;
					case ARMY:
						DEBUGOUTPUT("Army");
						push_input({ start_point, end_point, from_region.get_army().get_force() / 2, Operator::ArmyMove });
						break;
					case SCATTER_BOMB:
						DEBUGOUTPUT("Scatter Bomb");
						break;
					default:
						break;
					}
				}
				catch (std::exception e) {
					
				}
			}

		}
		break;
	case GLFW_KEY_G: // 快速生产
		if (GAMESTATUS::s_enable_control) {
			if (s_selected_gui.is_selected) {
				Point start_point = Point::to_point(s_selected_gui.grid);
				Point end_point = Point::to_point(INTERACTIVE::s_current_selected_grid);
				try {
					Region& from_region = RegionManager::instance_of().region(start_point.x, start_point.y);
					Region& to_region = RegionManager::instance_of().region(end_point.x, end_point.y);
					std::string result = "";
					switch (s_selected_weapon)
					{
					case NONE:
						GAMESOUND::play_error_sound();
						break;
					case NUCLEAR_MISSILE:
						if (from_region.get_owner() != 0) {
							s_selected_gui.message = u8"无效区块！";
							s_selected_gui.shake_gui(RENDERER::timer);
							break;
						}

						switch (s_nuclear_missile_level)
						{
						case 0:
							result = push_input_wait_for_result({ start_point, end_point, Operator::ProductWeapon0 });
							break;
						case 1:
							result = push_input_wait_for_result({ start_point, end_point, Operator::ProductWeapon1 });
							break;
						case 2:
							result = push_input_wait_for_result({ start_point, end_point, Operator::ProductWeapon2 });
							break;
						default:
							break;
						}
						if (result == "Success") {
							GAMESOUND::play_click_sound();
						}
						break;
					case ARMY:
						DEBUGOUTPUT("Army");
						result = push_input_wait_for_result({ start_point, end_point, 100, Operator::ProductArmy });
						if (result == "Success") {
							GAMESOUND::play_click_sound();
						}
						break;
					case SCATTER_BOMB:
						DEBUGOUTPUT("Scatter Bomb");
						break;
					default:
						break;
					}
				}
				catch (std::exception e) {

				}
			}

		}
		break;
	case GLFW_KEY_Z:
		if (GAMESTATUS::s_enable_control && s_selected_weapon == SCATTER_BOMB) {
			// 减小范围
			s_scatter_bomb_range = fmax(1, s_scatter_bomb_range - 1);
		}
		if (GAMESTATUS::s_enable_control && s_selected_weapon == NUCLEAR_MISSILE) {
			s_nuclear_missile_level = (s_nuclear_missile_level + 1) % 3;
		}
		break;
	case GLFW_KEY_X:
		if (GAMESTATUS::s_enable_control && s_selected_weapon == SCATTER_BOMB) {
			// 增大范围
			s_scatter_bomb_range = fmin(MAP::map_info.width() / 2, s_scatter_bomb_range + 1);
		}
		if (GAMESTATUS::s_enable_control && s_selected_weapon == NUCLEAR_MISSILE) {
			s_nuclear_missile_level = (s_nuclear_missile_level - 1 + 3) % 3;
		}
		break;
	case GLFW_KEY_1:
		if (GAMESTATUS::s_enable_control) {
			s_selected_weapon = NONE;
		}
		break;
	case GLFW_KEY_2:
		if (GAMESTATUS::s_enable_control) {
			s_selected_weapon = NUCLEAR_MISSILE;
		}
		break;
	case GLFW_KEY_3:
		if (GAMESTATUS::s_enable_control) {
			s_selected_weapon = ARMY;
		}
		break;
	case GLFW_KEY_4:
		if (GAMESTATUS::s_enable_control) {
		//	s_selected_weapon = SCATTER_BOMB;
		}
		break;
	default:
		break;
	}
}


bool compileShaders() {
	DEBUGOUTPUT("Compiling Shaders");
	PASS_MAIN::s_main_game_pass_program = CompileShader(main_game_pass_vert, main_game_pass_frag, nullptr, &PASS_MAIN::vertex_shader, &PASS_MAIN::fragment_shader, nullptr);
	if (PASS_MAIN::s_main_game_pass_program == -1) return false;
	PASS_NORMAL::s_normal_gl_program = CompileShader(normal_gl_vert, normal_gl_frag, nullptr, &PASS_NORMAL::normal_gl_vertex_shader, &PASS_NORMAL::normal_gl_fragment_shader, nullptr);
	if (PASS_NORMAL::s_normal_gl_program == -1) return false;
	PASS_MAP_RENDER::s_map_renderer_program = CompileShader(map_renderer_vert, map_renderer_frag, nullptr, &PASS_MAP_RENDER::map_renderer_vertex_shader, &PASS_MAP_RENDER::map_renderer_fragment_shader, nullptr);
	if (PASS_MAP_RENDER::s_map_renderer_program == -1) return false;
	PASS_BLOOM::s_gaussian_blur_program = CompileShader(gaussian_blur_vert, gaussian_blur_frag, nullptr, &PASS_BLOOM::gaussian_blur_vertex_shader, &PASS_BLOOM::gaussian_blur_fragment_shader, nullptr);
	if (PASS_BLOOM::s_gaussian_blur_program == -1) return false;
	PASS_DIRECT_TEX::s_direct_tex_program = CompileShader(direct_tex_pass_vert, direct_tex_pass_frag, nullptr, &PASS_DIRECT_TEX::direct_tex_vertex_shader, &PASS_DIRECT_TEX::direct_tex_fragment_shader, nullptr);
	if (PASS_DIRECT_TEX::s_direct_tex_program == -1) return false;
	PASS_POINT::s_points_program = CompileShader(point_renderer_vert, point_renderer_frag, nullptr, &PASS_POINT::points_vertex_shader, &PASS_POINT::points_fragment_shader, nullptr);
	DEBUGOUTPUT("Shaders Compiled");
	return true;
}

void init() {
	// 初始化
	// BASS
	
	GAMESOUND::init_bass();

	// 启用点精灵
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SPRITE);

	MAP::scale_map_camera.setMoveDuration(0.25);
	MAP::scale_map_camera.setPos(0, 0, -32,RENDERER::timer.time());
	MAP::camera.setMoveDuration(0.5);
	MAP::camera.setRotateDuration(0.25);
	MAP::camera.rotate(0, 0, -1.5, RENDERER::timer.time());
	MAP::camera.move(0, 0, -2, RENDERER::timer.time());
	MAP::map_rotation.set_start_position(-1, RENDERER::timer.time());
	MAP::map_rotation.set_total_duration(0.25);

	reset_camera();

	DEBUGOUTPUT("Building meshes..");

	DEBUGOUTPUT("Loading Models...");
	DEBUGOUTPUT("Loading Model: Missile");
	MODEL::MISSLE::missle_mesh.rgba(1, 1, 1, 1);
	MODEL::MISSLE::missle_mesh.load_from_obj("./resources/models/Missile AGM-65.obj");
	MODEL::MISSLE::missle_mesh.enable_color = true;
	MODEL::MISSLE::missle_mesh.enable_normal = true;
	MODEL::MISSLE::missle_mesh.enable_uv = true;

	MODEL::MISSLE::missle_mesh.build();

	DEBUGOUTPUT("Building Map Mesh...");
	MAP::s_mesh.append(1, -1, 0);
	MAP::s_mesh.append(1, 1, 0);
	MAP::s_mesh.append(-1, 1, 0);
	MAP::s_mesh.append(-1, -1, 0);
	MAP::s_mesh.build();

	MAP::map_mesh.append(1, -1, 0);
	MAP::map_mesh.append(1, 1, 0);
	MAP::map_mesh.append(-1, 1, 0);
	MAP::map_mesh.append(-1, -1, 0);
	MAP::map_mesh.build();
	DEBUGOUTPUT("Meshes built");

	PASS_POINT::point_renderer.init();


	DEBUGOUTPUT("Loading Textures...");
	TEXTURE::s_image_radioactive = LoadPNG("./resources/textures/radioactivity.png");
	TEXTURE::s_image_attack_target = LoadPNG("./resources/textures/target.png");
	TEXTURE::s_image_scatter = LoadPNG("./resources/textures/scatter.png");
	TEXTURE::s_image_lightning = LoadPNG("./resources/textures/lightning.png");
	TEXTURE::s_image_guard = LoadPNG("./resources/textures/guard.png");
	TEXTURE::s_image_forbid = LoadPNG("./resources/textures/forbid.png");
	TEXTURE::s_image_building_icon = LoadPNG("./resources/textures/buildings.png");
	MODEL::MISSLE::missle_texture = LoadPNG("./resources/models/fff.png", {}, true);
	DEBUGOUTPUT("Textures Loaded");
	RENDERER::timer.set_time(glfwGetTime());
	// 启动背景音乐
	GAMESOUND::play_background();
}

void destroy() {
	if (PASS_MAIN::s_main_game_pass_program != -1) glDeleteProgram(PASS_MAIN::s_main_game_pass_program);
	if (PASS_NORMAL::s_normal_gl_program != -1) glDeleteProgram(PASS_NORMAL::s_normal_gl_program);
	if (PASS_MAP_RENDER::s_map_renderer_program != -1) glDeleteProgram(PASS_MAP_RENDER::s_map_renderer_program);
	if (PASS_BLOOM::s_gaussian_blur_program != -1) glDeleteProgram(PASS_BLOOM::s_gaussian_blur_program);
	if (PASS_DIRECT_TEX::s_direct_tex_program != -1) glDeleteProgram(PASS_DIRECT_TEX::s_direct_tex_program);
	if (PASS_POINT::s_points_program != -1) glDeleteProgram(PASS_POINT::s_points_program);

	if (PASS_MAIN::vertex_shader != -1) glDeleteShader(PASS_MAIN::vertex_shader);
	if (PASS_MAIN::fragment_shader != -1) glDeleteShader(PASS_MAIN::fragment_shader);
	if (PASS_NORMAL::normal_gl_vertex_shader != -1) glDeleteShader(PASS_NORMAL::normal_gl_vertex_shader);
	if (PASS_NORMAL::normal_gl_fragment_shader != -1) glDeleteShader(PASS_NORMAL::normal_gl_fragment_shader);
	if (PASS_MAP_RENDER::map_renderer_vertex_shader != -1) glDeleteShader(PASS_MAP_RENDER::map_renderer_vertex_shader);
	if (PASS_MAP_RENDER::map_renderer_fragment_shader != -1) glDeleteShader(PASS_MAP_RENDER::map_renderer_fragment_shader);
	if (PASS_BLOOM::gaussian_blur_vertex_shader != -1) glDeleteShader(PASS_BLOOM::gaussian_blur_vertex_shader);
	if (PASS_BLOOM::gaussian_blur_fragment_shader != -1) glDeleteShader(PASS_BLOOM::gaussian_blur_fragment_shader);
	if (PASS_DIRECT_TEX::direct_tex_vertex_shader != -1) glDeleteShader(PASS_DIRECT_TEX::direct_tex_vertex_shader);
	if (PASS_DIRECT_TEX::direct_tex_fragment_shader != -1) glDeleteShader(PASS_DIRECT_TEX::direct_tex_fragment_shader);
	if (PASS_POINT::points_vertex_shader != -1) glDeleteShader(PASS_POINT::points_vertex_shader);
	if (PASS_POINT::points_fragment_shader != -1) glDeleteShader(PASS_POINT::points_fragment_shader);
	
	FBO::g_gaussian_blur_pass_fbo.release();
	FBO::g_gaussian_blur_vertical_pass_fbo.release();
	FBO::g_main_game_pass_fbo.release();
	FBO::g_final_mix_pass_fbo.release();
	FBO::g_flame_render_pass.release();
	FBO::g_point_render_pass_fbo.release();

	PASS_POINT::point_renderer.cleanup();

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
	if (TEXTURE::s_image_guard != GLFW_INVALID_VALUE) {
		glDeleteTextures(1, &TEXTURE::s_image_guard);
	}
	if (TEXTURE::s_image_forbid != GLFW_INVALID_VALUE) {
		glDeleteTextures(1, &TEXTURE::s_image_forbid);
	}
	if (TEXTURE::s_image_building_icon != GLFW_INVALID_VALUE) {
		glDeleteTextures(1, &TEXTURE::s_image_building_icon);
	}
}

int main() {
	srand(time(0));
	if (!glfwInit()) {
		DEBUGOUTPUT("Failed to initialize glfw"); return 0;
	}
	glfwSetErrorCallback((GLFWerrorfun)glfwErrorCallBack);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	GLFWmonitor* primary = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primary);

	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

#if defined(__aarch64__) || defined(__arm__)
	glfw_win = glfwCreateWindow(mode->width / 2, mode->height / 2, "MiniWar", primary, NULL);
#else
	#ifdef _DEBUG
	glfw_win = glfwCreateWindow(mode->width / 2, mode->height / 2, "MiniWar", 0, NULL);
	#else
	glfw_win = glfwCreateWindow(mode->width, mode->height, "MiniWar", primary, NULL);
	#endif
#endif

	glfwSetKeyCallback(glfw_win, (GLFWkeyfun)glfwKeyCallBack);
	glfwSetCursorPosCallback(glfw_win, (GLFWcursorposfun)glfwMouseCallback);
	glfwSetWindowSizeCallback(glfw_win, (GLFWwindowsizefun)glfwWindowSizeCallback);
	glfwSetScrollCallback(glfw_win, (GLFWscrollfun)glfwScrollCallback);

	glfwMakeContextCurrent(glfw_win);

	glfwSwapInterval(1);
	if (!glfw_win) {
		DEBUGOUTPUT("Failed to create window"); return 0;
	}



	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends

	const char* glsl_version = "#version 130";
	int err = glewInit();

	if (err) {
		DEBUGOUTPUT("Failed to initialize glew");
		DEBUGOUTPUT((char*)glewGetErrorString(err));
		goto destroy;
	}
	GLint float_framebuffer_supported;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &float_framebuffer_supported);
	if (!float_framebuffer_supported) {
		DEBUGOUTPUT("ARB_color_buffer_float not supported");
	}
	else {
		// 启用扩展
		glewExperimental = GL_TRUE;
		if (GLEW_ARB_color_buffer_float) {
			DEBUGOUTPUT("Activate ARB_color_buffer_float");
			glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);
			glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);
			glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);
		}
	}

#ifdef _DEBUG
	glDebugMessageCallback((GLDEBUGPROC)debugproc, 0);
	glEnable(GL_DEBUG_CALLBACK_FUNCTION);
#endif // DEBUG
	GLint maxTexSize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
	DEBUGOUTPUT("Max Texture Size:", std::to_string(maxTexSize));

	io.Fonts->TexDesiredWidth = maxTexSize;

	io.Fonts->Clear();
	//io.Fonts->AddFontDefault();
	UIFonts::default_font = io.Fonts->AddFontFromFileTTF("./resources/fonts/msyh.ttf", 32.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
	UIFonts::large_font = io.Fonts->AddFontFromFileTTF("./resources/fonts/msyh.ttf", 48.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
	UIFonts::menu_font = io.Fonts->AddFontFromFileTTF("./resources/fonts/msyh.ttf", 64.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
	if (!io.Fonts->Build()) {
		DEBUGOUTPUT("Failed to build fonts");
		goto destroy;
	}
	ImGui_ImplGlfw_InitForOpenGL(glfw_win, true);

	ImGui_ImplOpenGL3_Init(glsl_version);
	DEBUGOUTPUT("OpenGL Version", (std::string)(char*)glGetString(GL_VERSION));
	DEBUGOUTPUT("GLSL Version", (std::string)(char*)glGetString(GL_SHADING_LANGUAGE_VERSION));


	int width, height;
	glfwGetFramebufferSize(glfw_win, &width, &height);
	FBO::g_main_game_pass_fbo = FragmentBuffer(width, height, GL_RGBA16F, true);
	FBO::g_final_mix_pass_fbo = FragmentBuffer(width, height, GL_RGBA16F);
	FBO::g_gaussian_blur_pass_fbo = FragmentBuffer(width, height, GL_RGBA16F, true);
	FBO::g_gaussian_blur_vertical_pass_fbo = FragmentBuffer(width, height, GL_RGBA16F, true);
	FBO::g_point_render_pass_fbo = FragmentBuffer(width, height, GL_RGBA16F);
	FBO::g_flame_render_pass = FlameRenderPass(width, height, GL_RGBA16F);
	FBO::g_flame_render_pass.init();


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
		RENDERER::timer.set_time(glfwGetTime());
		prepare_render();
		KeyProcess();
		render();
		glUseProgram(0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		try {
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			render_imgui(io);


			// Rendering
			glDebugMessageCallback(0, 0);
			ImGui::Render();
			glDebugMessageCallback((GLDEBUGPROC)debugproc, 0);
			glDebugMessageCallback(0, 0);
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			glDebugMessageCallback((GLDEBUGPROC)debugproc, 0);
			ImGui::EndFrame();
		}
		catch (const std::exception& e) {
			DEBUGOUTPUT("Exception in ImGui", e.what());
		}
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
	DEBUGOUTPUT(str);
}
void glfwKeyCallBack(GLFWwindow* window, int key, int scanmode, int action, int mods) {
	INTERACTIVE::keys[key] = (bool)action;
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
	if (s_menu_gui.is_open() || !GAMESTATUS::s_in_game || !GAMESTATUS::s_enable_control) { 
		return;
	}
	MAP::scale_map_camera.move(0, 0, yoffset, RENDERER::timer.time());
	MAP::camera.rotate_to(0, 0, -1.2 /(1 + 2 * exp(-0.05 * MAP::scale_map_camera.getZ() * MAP::scale_map_camera.getZ())), RENDERER::timer.time());
	MAP::map_rotation.new_end_position(-(exp(-0.01 * pow(MAP::scale_map_camera.getZ() * MAP::scale_map_camera.getZ(),2))),RENDERER::timer.time());
}

void glfwWindowSizeCallback(GLFWwindow* window, int width, int height) {
	try {
		glViewport(0, 0, width, height);
		FBO::g_main_game_pass_fbo.resize(width, height, true);
		FBO::g_gaussian_blur_pass_fbo.resize(width, height, true);
		FBO::g_gaussian_blur_vertical_pass_fbo.resize(width, height, true);
		FBO::g_final_mix_pass_fbo.resize(width, height);
		FBO::g_point_render_pass_fbo.resize(width, height);
		FBO::g_flame_render_pass.get_fbo().resize(width, height);
		s_pause_rendering = false;

		ImGuiIO& io = ImGui::GetIO();

		if (height >= 800) {
			s_dpi_scale = 1;
		}
		else if (height >= 600) {
			s_dpi_scale = 0.75;
		}
		else {
			s_dpi_scale = 0.5;
		}

		// 方法1: 设置全局字体缩放
		io.FontGlobalScale = s_dpi_scale;

		// 方法2: 修改整体Style缩放
		ImGuiStyle& style = ImGui::GetStyle();
		style.ScaleAllSizes(s_dpi_scale);
		style.WindowMinSize = ImVec2(
			std::fmax(1.0f, style.WindowMinSize.x),
			std::fmax(1.0f, style.WindowMinSize.y)
		);
		style.ScrollbarSize = std::fmax(1.0f, style.ScrollbarSize);
		style.GrabMinSize = std::fmax(1.0f, style.GrabMinSize);
		style.WindowBorderSize = std::fmax(0.0f, style.WindowBorderSize);
		style.FrameBorderSize = std::fmax(0.0f, style.FrameBorderSize);
		style.TabBorderSize = std::fmax(0.0f, style.TabBorderSize);
		style.IndentSpacing = std::fmax(0.0f, style.IndentSpacing);
		style.ScrollbarRounding = std::fmax(0.0f, style.ScrollbarRounding);
		style.GrabRounding = std::fmax(0.0f, style.GrabRounding);
		style.WindowRounding = (style.WindowRounding < 0.0f || style.WindowRounding > 14.0f) ? 0.0f : style.WindowRounding;
		style.FrameRounding = (style.FrameRounding < 0.0f || style.FrameRounding > 14.0f) ? 0.0f : style.FrameRounding;
		style.PopupRounding = (style.PopupRounding < 0.0f || style.PopupRounding > 14.0f) ? 0.0f : style.PopupRounding;
		style.ChildRounding = (style.ChildRounding < 0.0f || style.ChildRounding > 14.0f) ? 0.0f : style.ChildRounding;
		
	}
	catch (const char* str) {
		s_pause_rendering = true;
	}
}