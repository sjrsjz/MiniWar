#pragma once


#ifndef _output_h
#define  _output_h
#include <iostream>
#ifndef _gl_h
#define  _gl_h
#include "../include/GL/gl.h"
#endif // !_gl_h

void println(const char* str);
void debugProc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

#endif
