﻿#pragma once
#include "../../include/linmath.h"
#include "RegionData.h"
#include <tuple>
#include <cmath>

class RegionSelector
{
private:
    float m_fov;
    float m_width, m_height;
    mat4x4 m_view_mat;
    mat4x4 m_model_mat;
    int m_map_width, m_map_height; 
   
    RegionData* m_regions;

private:
	inline RegionData empty_region() {
		RegionData region;
		region.cell_center_x = 0;
		region.cell_center_y = 0;
        region.army_position_x = 0;
		region.army_position_y = 0;
		region.identity = 0;
		region.region_additional_info = 0;
		return region;
	}
	inline RegionData get_region(int x, int y) {
        if (x < 0 || x >= m_map_width || y < 0 || y >= m_map_height)
            return empty_region();
        return m_regions[y * m_map_width + x];
	}
    inline std::tuple<int, int, float> cell(vec2 cell_uv, vec2 cell_center[3][3]) {
		float min_dist = 1e9;
		vec2 idx = { 0, 0 };
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                vec2 offset;
                vec2_sub(offset, cell_uv, cell_center[i][j]);
                offset[0] -= i - 1;
                offset[1] -= j - 1;
                float dist = vec2_len(offset);
                if (dist < min_dist) {
                    min_dist = dist;
                    idx[0] = i;
                    idx[1] = j;
                }
            }
        }
		return { idx[0] - 1, idx[1] - 1, min_dist };
    }
    inline std::tuple<int, int, float, float> uv_to_cell_position(float u, float v) {
		u = (0.5 * u + 0.5) * m_map_width;
		v = (0.5 * v + 0.5) * m_map_height;
		int x = (int)u;
		int y = (int)v;
		float cell_u = u - x;
		float cell_v = v - y;
		return { x, y, cell_u, cell_v };
    }


public:
    inline std::tuple<float, float, float> intersect_plane(float mouseX, float mouseY) {
        // NDC坐标转换
        float ndcX = (2.0f * mouseX / this->m_width) - 1.0f;
        float ndcY = 1.0f - (2.0f * mouseY / this->m_height);

        // 射线方向计算
        float aspect = this->m_width / this->m_height;
        vec3 dir = { ndcX * aspect, ndcY , this->m_fov };
        vec3_norm(dir, dir);

        vec4 dirWorld;
		float dirW[4] = { dir[0], dir[1], dir[2], 0.0f };
        mat4x4_mul_vec4(dirWorld, this->m_view_mat, dirW);

        // 射线原点
        vec4 rayOrigin;
		vec4 zero = { 0,0,0,1.0f };
        mat4x4_mul_vec4(rayOrigin, this->m_view_mat, zero);

        // 平面参数
        vec4 planeU, planeV, planePos;
		vec4 u0 = { 1,0,0,0 };
		vec4 v0 = { 0,0,1,0 };
		vec4 pos0 = { 0,0,0,1 };
        mat4x4_mul_vec4(planeU, this->m_model_mat, u0);
        mat4x4_mul_vec4(planeV, this->m_model_mat, v0);
        mat4x4_mul_vec4(planePos, this->m_model_mat, pos0);

        // 平面法向量
        vec3 normal;
		vec3 planeU3 = { planeU[0],planeU[1],planeU[2] };
		vec3 planeV3 = { planeV[0],planeV[1],planeV[2] };
		vec3_mul_cross(normal, planeU3, planeV3);

        // 求交
        vec3 po = {
            planePos[0] - rayOrigin[0],
            planePos[1] - rayOrigin[1],
            planePos[2] - rayOrigin[2]
        };

        float t = vec3_mul_inner(normal, po);
		vec3 dirWorld3 = { dirWorld[0],dirWorld[1],dirWorld[2] };
        float temp = vec3_mul_inner(normal, dirWorld3);
        t /= temp;

        if (t < 0) return { -1, 0, 0 };

        // 交点
        vec3 pos = {
            rayOrigin[0] + dirWorld[0] * t,
            rayOrigin[1] + dirWorld[1] * t,
            rayOrigin[2] + dirWorld[2] * t
        };

        // UV计算
        vec3 posLocal = {
            pos[0] - planePos[0],
            pos[1] - planePos[1],
            pos[2] - planePos[2]
        };

		float u = vec3_mul_inner(posLocal, planeU3);
		float v = vec3_mul_inner(posLocal, planeV3);

        return { u, v, t };
    }

	inline std::tuple<bool, int, int> select_region(float mouseX, float mouseY) {
		auto [u, v, t] = intersect_plane(mouseX, mouseY);

		if (abs(u)>1 || abs(v)>1) return { false, -1, -1 };

		auto [x, y, cell_u, cell_v] = uv_to_cell_position(u, v);
		vec2 cell_center[3][3];
		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				RegionData cell = get_region(x + i, y + j);
				cell_center[i + 1][j + 1][0] = cell.cell_center_x;
				cell_center[i + 1][j + 1][1] = cell.cell_center_y;
			}
		}
		vec2 cell_uv = { cell_u, cell_v };
		auto [idx_x, idx_y, min_dist] = cell(cell_uv, cell_center);
		if (x + idx_x < 0 || x + idx_x >= m_map_width || y + idx_y < 0 || y + idx_y >= m_map_height)
			return { false, -1, -1 };
		return { true, x + idx_x, y + idx_y };
	}

		

    inline std::tuple<bool, int, int> operator()(float mouseX, float mouseY) {
		return select_region(mouseX, mouseY);
    }
    inline RegionSelector(float fov, float width, float height, mat4x4 viewMat, mat4x4 modelMat, int mapWidth, int mapHeight, void* regions) {
		this->m_fov = fov;
		this->m_width = width;
		this->m_height = height;
		
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				this->m_view_mat[i][j] = viewMat[i][j];
				this->m_model_mat[i][j] = modelMat[i][j];
			}
		}

		this->m_map_width = mapWidth;
		this->m_map_height = mapHeight;
		this->m_regions = (RegionData*)regions;
	}
};