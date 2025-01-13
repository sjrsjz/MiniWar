#pragma once
#include <iostream>
#include "../include/GL/glew.h"
#include "debug.h"

void debugproc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
