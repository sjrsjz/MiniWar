#pragma once
#include "FragmentBuffer.h"
#include "../Timer.h"
#include "../mash.h"
#include "../../shaders/flame_render_pass.frag"
#include "../../shaders/flame_render_pass.vert"
#include "../shader.h"
#include "../../include/linmath.h"

class FlameRenderPass
{
private:
	FragmentBuffer m_fbo;
	GLuint m_program = 0;
	GLuint m_vertex_shader = 0;
	GLuint m_fragment_shader = 0;
public:
	FlameRenderPass(int width, int height, int data_format, bool mipmap = false)
		: m_fbo(width, height, data_format, mipmap)
	{
	}
	FlameRenderPass()
	{
	}
	~FlameRenderPass()
	{
	}

	inline void init()
	{
		m_program = CompileShader(flame_render_pass_vert, flame_render_pass_frag, nullptr, &m_vertex_shader, &m_fragment_shader, nullptr);
	}

	inline void release() {
		glDeleteProgram(m_program);
		glDeleteShader(m_vertex_shader);
		glDeleteShader(m_fragment_shader);
		m_fbo.release();
	}

	inline FragmentBuffer& get_fbo()
	{
		return m_fbo;
	}
	inline GLuint get_texture() const
	{
		return m_fbo.get_texture();
	}
	inline void bind_frameBuffer()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo.get_texture());
	}
	inline void unbind_frameBuffer()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	inline void render(GLuint mix_tex, const Timer& timer, const mash& quad, float mix_strength) {
		m_fbo.bind_frameBuffer();
		mat4x4 m, p, mvp;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		float ratio = 1;
		int mvp_location = glGetUniformLocation(m_program, "MVP");
		mat4x4_identity(m);
		mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		mat4x4_mul(mvp, p, m);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mix_tex);
		glUseProgram(m_program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);

		glUniform1f(glGetUniformLocation(m_program, "iTime"), timer.time());
		glUniform1i(glGetUniformLocation(m_program, "iChannel0"), 0);
		glUniform2f(glGetUniformLocation(m_program, "iResolution"), m_fbo.width(), m_fbo.height());
		glUniform1f(glGetUniformLocation(m_program, "mix_strength"), mix_strength);

		quad.render(m_program, "vPos", nullptr, nullptr, nullptr);

		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);
		m_fbo.unbind_frameBuffer();
	}
};
