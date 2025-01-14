#pragma once
#include "../include/GL/glew.h"
#include <vector>
#include <stdexcept>
#include <memory>

struct vertex {
    float x, y, z;
    float r, g, b, a;
    float u, v;
    float n_x, n_y, n_z;
};

class mash {
public:
    float r = 0.f, g = 0.f, b = 0.f, a = 0.f;
    float u = 0.f, v = 0.f;
    float n_x = 0.f, n_y = 0.f, n_z = 0.f;
    GLuint vertex_buffer;
    std::vector<vertex> vertexs;
    bool enable_color = false, enable_uv = false, enable_normal = false;

    mash() : vertex_buffer(0) {}

    void append(float x, float y, float z) {
        vertex v_;
        v_.x = x; v_.y = y; v_.z = z; v_.r = r; v_.g = g; v_.b = b; v_.a = a; v_.u = u; v_.v = v;
        v_.n_x = n_x; v_.n_y = n_y; v_.n_z = n_z;
        vertexs.push_back(v_);
    }

    void build() {
        int size0 = (3 + 4 * (int)enable_color + 2 * (int)enable_uv + 3 * (int)enable_normal) * sizeof(float);
        size_t size = vertexs.size() * size0;
        std::unique_ptr<float[]> buffer(new float[size / sizeof(float)]);

        unsigned int j = 0;
        for (const auto& v : vertexs) {
            buffer[j++] = v.x;
            buffer[j++] = v.y;
            buffer[j++] = v.z;
            if (enable_color) {
                buffer[j++] = v.r;
                buffer[j++] = v.g;
                buffer[j++] = v.b;
                buffer[j++] = v.a;
            }
            if (enable_uv) {
                buffer[j++] = v.u;
                buffer[j++] = v.v;
            }
            if (enable_normal) {
                buffer[j++] = v.n_x;
                buffer[j++] = v.n_y;
                buffer[j++] = v.n_z;
            }
        }

        if (vertex_buffer != 0) {
            glDeleteBuffers(1, &vertex_buffer);
        }
        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, size, buffer.get(), GL_STATIC_DRAW);


        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void render(GLuint program, const GLchar* pos, const GLchar* color, const GLchar* uv, const GLchar* normal) const{
        //int location = 0;
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        int size0 = (3 + 4 * (int)enable_color + 2 * (int)enable_uv + 3 * (int)enable_normal) * sizeof(float);
        unsigned int j = 0;
        GLuint vpos_location = glGetAttribLocation(program, pos);
        //GLuint vpos_location = location++;

        j = 0;
        glEnableVertexAttribArray(vpos_location);
        glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, size0, (void*)0);
        j += sizeof(float) * 3;

		GLuint vcol_location = 0, vuv_location = 0, vnormal_location = 0;

        if (enable_color) {
            vcol_location = glGetAttribLocation(program, color);
            if (vcol_location == -1) throw std::runtime_error("color attribute not found");
            //vcol_location = location++;
            glEnableVertexAttribArray(vcol_location);
            glVertexAttribPointer(vcol_location, 4, GL_FLOAT, GL_FALSE, size0, (void*)j);
            j += sizeof(float) * 4;
        }
        if (enable_uv) {
            vuv_location = glGetAttribLocation(program, uv);
            if (vuv_location == -1) throw std::runtime_error("uv attribute not found");
            //vuv_location = location++;
            glEnableVertexAttribArray(vuv_location);
            glVertexAttribPointer(vuv_location, 2, GL_FLOAT, GL_FALSE, size0, (void*)j);
            j += sizeof(float) * 2;
        }
        if (enable_normal) {
            vnormal_location = glGetAttribLocation(program, normal);
			if (vnormal_location == -1) throw std::runtime_error("normal attribute not found");
			//vnormal_location = location++;
            glEnableVertexAttribArray(vnormal_location);
            glVertexAttribPointer(vnormal_location, 3, GL_FLOAT, GL_FALSE, size0, (void*)j);
            j += sizeof(float) * 3;
        }

		glDrawArrays(GL_QUADS, 0, vertexs.size());


		glDisableVertexAttribArray(vpos_location);
        if (enable_color) {
            glDisableVertexAttribArray(vcol_location);
        }
		if (enable_uv) {
			glDisableVertexAttribArray(vuv_location);
		}
        if (enable_normal) {
            glDisableVertexAttribArray(vnormal_location);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }


    void rgba(float r, float g, float b, float a) {
        this->r = r; this->g = g; this->b = b; this->a = a;
    }

    void UV(float u, float v) {
        this->u = u; this->v = v;
    }

    void normal(float n_x, float n_y, float n_z) {
        this->n_x = n_x; this->n_y = n_y; this->n_z = n_z;
    }

    ~mash() {
        if (vertex_buffer != 0) {
            glDeleteBuffers(1, &vertex_buffer);
        }
    }
};
