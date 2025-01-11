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
}