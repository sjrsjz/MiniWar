#include "../header/output.h"
void debugproc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	DEBUGOUTPUT(message);
}
