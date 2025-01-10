

#include "../header/shader.h"
int CompileShader(const char* vertex_shader_text, const char* fragment_shader_text, const char* geometry_shader_text, GLuint* vertex_shader, GLuint* fragment_shader, GLuint* geometry_shader) {
	int program; bool err = false;

	if (vertex_shader_text != nullptr) {
		*vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(*vertex_shader, 1, &vertex_shader_text, NULL);
		glCompileShader(*vertex_shader);
		GLint a = false;
		glGetShaderiv(*vertex_shader, 0x8B81, &a);
		if (!a)
		{
			char buf[2048]; int err_size;
			glGetShaderInfoLog(*vertex_shader, 2047, &err_size, (char*)buf);
			println(buf); err |= true;
		}
	}
	if (geometry_shader_text != nullptr) {
		*geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(*geometry_shader, 1, &geometry_shader_text, NULL);
		glCompileShader(*geometry_shader);
		GLint a = false;
		glGetShaderiv(*geometry_shader, 0x8B81, &a);
		if (!a)
		{
			char buf[2048]; int err_size;
			glGetShaderInfoLog(*geometry_shader, 2047, &err_size, (char*)buf);
			println(buf); err |= true;
		}
	}
	if (vertex_shader_text != nullptr) {
		*fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(*fragment_shader, 1, &fragment_shader_text, NULL);
		glCompileShader(*fragment_shader);
		GLint a = false;
		glGetShaderiv(*fragment_shader, 0x8B81, &a);
		if (!a)
		{
			char buf[2048]; int err_size;
			glGetShaderInfoLog(*fragment_shader, 2047, &err_size, (char*)buf);
			println(buf); err |= true;
		}
	}
	program = glCreateProgram();
	if (vertex_shader_text != nullptr) glAttachShader(program, *vertex_shader);
	if (fragment_shader_text != nullptr) glAttachShader(program, *fragment_shader);
	if (geometry_shader_text != nullptr) glAttachShader(program, *geometry_shader);
	glLinkProgram(program);
	return err ? -1 : program;

}
int CompileComputeShader(const char* compute_shader_text, GLuint* compute_shader) {
	int program; bool err = false;
	if (compute_shader_text == nullptr) return -1;

	if (compute_shader_text != nullptr) {
		*compute_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(*compute_shader, 1, &compute_shader_text, NULL);
		glCompileShader(*compute_shader);
		GLint a = false;
		glGetShaderiv(*compute_shader, 0x8B81, &a);
		if (!a)
		{
			char buf[2048]; int err_size;
			glGetShaderInfoLog(*compute_shader, 2047, &err_size, (char*)buf);
			println(buf); err |= true;
		}
	}
	program = glCreateProgram();
	glAttachShader(program, *compute_shader);
	glLinkProgram(program);
	return err ? -1 : program;

}