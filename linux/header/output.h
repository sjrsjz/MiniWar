#ifndef _output_h
#define  _output_h
#include <iostream>
#include "../include/GL/glew.h"


void println(const char* str);
void debugProc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

#endif
