#pragma once
#include "../include/GL/glew.h"
#include "output.h"
int CompileShader(const char* vertex_shader_text, const char* fragment_shader_text, const char* geometry_shader_text, GLuint* vertex_shader, GLuint* fragment_shader, GLuint* geometry_shader);
int CompileComputeShader(const char* compute_shader_text, GLuint* compute_shader);