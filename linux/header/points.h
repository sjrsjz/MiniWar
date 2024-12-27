#include "../include/GL/glew.h"
#include <vector>
struct Vertex {
    float x, y, z;    // vPos
    float r, g, b, a; // vColor
};

class PointRenderer {
private:
    GLuint vbo;
    std::vector<Vertex> vertices;

public:
    void init() {

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
            vertices.size() * sizeof(Vertex),
            vertices.data(),
            GL_DYNAMIC_DRAW);


        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void update(const std::vector<Vertex>& new_vertices) {

        vertices = new_vertices;
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
            vertices.size() * sizeof(Vertex),
            vertices.data(),
            GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void render(GLuint program) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        // 位置属性 - vPos
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
            sizeof(Vertex),
            (void*)offsetof(Vertex, x));

        //// 颜色属性 - vColor
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
            sizeof(Vertex),
            (void*)offsetof(Vertex, r));
        glPointSize(20.0f);
        glDrawArrays(GL_POINTS, 0, vertices.size());
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);


    }

    void cleanup() {
        glDeleteBuffers(1, &vbo);
    }
};