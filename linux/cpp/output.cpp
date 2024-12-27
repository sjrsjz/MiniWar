#include "../header/output.h"
void println(const char* str) {
	std::cout << str << std::endl;
}
void debugProc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	println(message);
}
