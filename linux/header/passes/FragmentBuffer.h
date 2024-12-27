#ifndef FRAGMENT_BUFFER_H
#define FRAGMENT_BUFFER_H
#include "../../include/GL/glew.h"

class FragmentBuffer {
private:
    GLuint m_fbo;
    GLuint m_texture;
    GLuint m_depthBuffer;
    int m_width;
    int m_height;
    int m_data_format;

public:
    inline FragmentBuffer()
        : m_fbo(0), m_texture(0), m_depthBuffer(0), m_width(0), m_height(0), m_data_format(GL_RGB) {
    }
    inline FragmentBuffer(int width, int height, int data_format, bool mipmap = false)
        : m_fbo(0), m_texture(0), m_depthBuffer(0), m_width(width), m_height(height), m_data_format(data_format) {
        create_frameBuffer();
        create_texture(m_width, m_height, mipmap);
        create_depthBuffer(m_width, m_height);
        check_frameBuffer();
        unbind_frameBuffer();
    }

    inline ~FragmentBuffer() {
    }
    inline GLuint get_texture() const {
        return m_texture;
    }

    // 创建帧缓冲
    inline void create_frameBuffer() {
        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
    }

    // 创建纹理
    inline void create_texture(int width, int height, bool mipmap = false) {

        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, m_data_format, width, height, 0, GL_RGBA, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 禁用颜色值钳制
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
    }

    // 创建深度缓冲
    inline void create_depthBuffer(int width, int height) {
        glGenRenderbuffers(1, &m_depthBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer);
    }

    // 检查帧缓冲是否完整
    inline void check_frameBuffer() {
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
        }
    }

    // 绑定帧缓冲
    inline void bind_frameBuffer() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    }

    // 解绑帧缓冲
    inline void unbind_frameBuffer() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // 绑定纹理
    inline void bind_texture() {
        glBindTexture(GL_TEXTURE_2D, m_texture);
    }

    // 重新设置大小
    inline void resize(int width, int height, bool mipmap = false) {
        m_width = width;
        m_height = height;
        bind_frameBuffer();
        create_texture(m_width, m_height, mipmap);
        create_depthBuffer(m_width, m_height);
        check_frameBuffer();
        unbind_frameBuffer();
    }

    // 释放资源
    inline void release() {
        if (m_fbo) {
            glDeleteFramebuffers(1, &m_fbo);
            m_fbo = 0;
        }
        if (m_texture) {
            glDeleteTextures(1, &m_texture);
            m_texture = 0;
        }
        if (m_depthBuffer) {
            glDeleteRenderbuffers(1, &m_depthBuffer);
            m_depthBuffer = 0;
        }
    }
    inline int width() const {
        return m_width;
    }
    inline int height() const {
        return m_height;
    }
};
#endif