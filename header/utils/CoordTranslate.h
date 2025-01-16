#include "../../include/linmath.h"

namespace CoordTranslate {
    inline void project(mat4x4& projection, float W, float H, float fov) {
        float ratio = W / (float)H;
        mat4x4_identity(projection);
        projection[0][0] = 1 / ratio * fov;
        projection[1][1] = fov;
        projection[2][2] = -1;
        projection[3][3] = 0;
        projection[2][3] = -1;
    }

    inline void world_to_screen(vec2& screen_pos, vec3& vPos, mat4x4& mvp, mat4x4& g_model_trans_mat_inv, mat4x4& g_trans_mat) {
        /*
            vec4 wp = inverse(g_model_trans_mat_inv * g_trans_mat) * vec4(vPos, 1.0);
            gl_Position = MVP  * wp;
        */

        mat4x4 tmp;
        mat4x4 inverse;
        vec4 wp = { vPos[0], vPos[1], vPos[2], 1.0 };
        vec4 tmp_v;
        mat4x4_mul(tmp, g_model_trans_mat_inv, g_trans_mat);
        mat4x4_invert(inverse, tmp);
        mat4x4_mul_vec4(tmp_v, inverse, wp);
        mat4x4_mul_vec4(wp, mvp, tmp_v);

        vec4_scale(tmp_v, wp, 1.0 / wp[3]);

        screen_pos[0] = tmp_v[0] * 0.5 + 0.5;
        screen_pos[1] = 0.5 - tmp_v[1] * 0.5;
    }
    inline void mat4x4_align_x_to(mat4x4& M, vec3 dir) {
        // 归一化目标方向
        float len = sqrtf(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);
        dir[0] /= len;
        dir[1] /= len;
        dir[2] /= len;

        // x轴单位向量
        vec3 x_axis = { 1, 0, 0 };

        // 计算旋转轴 (cross product)
        vec3 rotation_axis;
        rotation_axis[0] = x_axis[1] * dir[2] - x_axis[2] * dir[1];
        rotation_axis[1] = x_axis[2] * dir[0] - x_axis[0] * dir[2];
        rotation_axis[2] = x_axis[0] * dir[1] - x_axis[1] * dir[0];

        // 归一化旋转轴
        len = sqrtf(rotation_axis[0] * rotation_axis[0] +
            rotation_axis[1] * rotation_axis[1] +
            rotation_axis[2] * rotation_axis[2]);

        // 如果旋转轴长度接近0，说明向量平行
        if (len < 1e-6f) {
            mat4x4_identity(M);
            // 如果方向相反，绕任意垂直轴旋转180度
            if (dir[0] < 0) {
                M[0][0] = -1;
                M[1][1] = -1;
            }
            return;
        }

        rotation_axis[0] /= len;
        rotation_axis[1] /= len;
        rotation_axis[2] /= len;

        // 计算旋转角度 (dot product)
        float cos_theta = dir[0]; // dot product with (1,0,0)
        float sin_theta = sqrtf(1 - cos_theta * cos_theta);

        // 使用罗德里格斯旋转公式
        float u = rotation_axis[0];
        float v = rotation_axis[1];
        float w = rotation_axis[2];
        float a = 1.0f - cos_theta;

        M[0][0] = cos_theta + u * u * a;
        M[1][0] = u * v * a - w * sin_theta;
        M[2][0] = u * w * a + v * sin_theta;
        M[3][0] = 0;

        M[0][1] = u * v * a + w * sin_theta;
        M[1][1] = cos_theta + v * v * a;
        M[2][1] = v * w * a - u * sin_theta;
        M[3][1] = 0;

        M[0][2] = u * w * a - v * sin_theta;
        M[1][2] = v * w * a + u * sin_theta;
        M[2][2] = cos_theta + w * w * a;
        M[3][2] = 0;

        M[0][3] = 0;
        M[1][3] = 0;
        M[2][3] = 0;
        M[3][3] = 1;
    }
}