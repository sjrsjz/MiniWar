#pragma once
#include "../../include/GL/glew.h"
#include "SSBOByteArray.h"
class SSBO {

private:
    GLuint m_ssbo;
    GLuint m_binding_point_index;
    GLuint m_size;
    GLuint m_usage;

public:
    inline SSBO()
        : m_ssbo(0), m_binding_point_index(0), m_size(0), m_usage(0)
    {
    }

    inline SSBO(GLuint size, GLuint usage)
        : m_ssbo(0), m_binding_point_index(0), m_size(size), m_usage(usage)
    {
        create_ssbo();
    }

    inline SSBO(SSBO& o) {
		// Copy constructor
		// 新建一个SSBO对象，然后将o的数据拷贝到新对象中
		if (this == &o) {
			return;
		}
        release();
        m_binding_point_index = o.m_binding_point_index;
		m_size = o.m_size;
		m_usage = o.m_usage;


		create_ssbo();
		o.bind_ssbo();
	    
		// 从o中读取数据
		void* data = new char[m_size];
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_size, data);
		update_data(data, m_size);

		o.unbind_ssbo();
		delete[] data;

    }

	inline SSBO& operator=(const SSBO& o) {
		// Copy assignment
		// 释放当前对象的资源，然后将o的数据拷贝到当前对象中
		if (this != &o) {
			release();
			m_binding_point_index = o.m_binding_point_index;
			m_size = o.m_size;
			m_usage = o.m_usage;

			create_ssbo();
			o.bind_ssbo();

			// 从o中读取数据
			void* data = new char[m_size];
			glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_size, data);
			update_data(data, m_size);

			o.unbind_ssbo();
			delete[] data;

		}
		return *this;
	}

	inline SSBO(const SSBO& o) {
		// move constructor
		// 将o的数据移动到当前对象中
		m_ssbo = o.m_ssbo;
		m_binding_point_index = o.m_binding_point_index;
		m_size = o.m_size;
		m_usage = o.m_usage;
	}

    inline ~SSBO()
    {
        release();
    }

    inline void create_ssbo()
    {
        glGenBuffers(1, &m_ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, m_size, nullptr, m_usage);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_binding_point_index, m_ssbo);
    }

    inline void bind_ssbo() const
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
    }

	inline void bind(GLuint index) const
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, m_ssbo);
	}

    inline void unbind_ssbo() const
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    inline void set_binding_point_index(GLuint index)
    {
        m_binding_point_index = index;
    }

    inline void set_size(GLuint size)
    {
        m_size = size;
    }

    inline void set_usage(GLuint usage)
    {
        m_usage = usage;
    }

    inline GLuint get_ssbo() const
    {
        return m_ssbo;
    }

    inline GLuint get_binding_point_index() const
    {
        return m_binding_point_index;
    }

    inline GLuint get_size() const
    {
        return m_size;
    }

    inline GLuint get_usage() const
    {
        return m_usage;
    }

    inline void update_data(const void* data, GLuint size) {
        bind_ssbo();
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, data);
        unbind_ssbo();
    }

    inline void release()
    {
        if (m_ssbo != 0) {
            glDeleteBuffers(1, &m_ssbo);
            m_ssbo = 0;
        }
    }
};