#pragma once
#include "../../include/linmath.h"
#include "RegionData.h"
#include <tuple>
#include <cmath>

class RegionSelector
{
private:
    float fov;
    float width, height;
    mat4x4 viewMat;
    mat4x4 modelMat;
    int mapWidth, mapHeight; 
   
    RegionData* regions;

private:
	inline RegionData empty_region() {
		RegionData region;
		region.cell_center_x = 0;
		region.cell_center_y = 0;
        region.army_position_x = 0;
		region.army_position_y = 0;
		region.identity = 0;
		region.is_capital = 0;
		return region;
	}
	inline RegionData getRegion(int x, int y) {
        if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight)
            return empty_region();
        return regions[y * mapWidth + x];
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
		u = (0.5 * u + 0.5) * mapWidth;
		v = (0.5 * v + 0.5) * mapHeight;
		int x = (int)u;
		int y = (int)v;
		float cell_u = u - x;
		float cell_v = v - y;
		return { x, y, cell_u, cell_v };
    }


public:
    inline std::tuple<float, float, float> intersectPlane(float mouseX, float mouseY) {
        // NDC坐标转换
        float ndcX = (2.0f * mouseX / this->width) - 1.0f;
        float ndcY = 1.0f - (2.0f * mouseY / this->height);

        // 射线方向计算
        float aspect = this->width / this->height;
        vec3 dir = { ndcX * aspect, ndcY , this->fov };
        vec3_norm(dir, dir);

        vec4 dirWorld;
		float dirW[4] = { dir[0], dir[1], dir[2], 0.0f };
        mat4x4_mul_vec4(dirWorld, this->viewMat, dirW);

        // 射线原点
        vec4 rayOrigin;
		vec4 zero = { 0,0,0,1.0f };
        mat4x4_mul_vec4(rayOrigin, this->viewMat, zero);

        // 平面参数
        vec4 planeU, planeV, planePos;
		vec4 u0 = { 1,0,0,0 };
		vec4 v0 = { 0,0,1,0 };
		vec4 pos0 = { 0,0,0,1 };
        mat4x4_mul_vec4(planeU, this->modelMat, u0);
        mat4x4_mul_vec4(planeV, this->modelMat, v0);
        mat4x4_mul_vec4(planePos, this->modelMat, pos0);

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

	inline std::tuple<bool, int, int> selectRegion(float mouseX, float mouseY) {
		auto [u, v, t] = intersectPlane(mouseX, mouseY);

		if (abs(u)>1 || abs(v)>1) return { false, -1, -1 };

		auto [x, y, cell_u, cell_v] = uv_to_cell_position(u, v);
		vec2 cell_center[3][3];
		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				RegionData cell = getRegion(x + i, y + j);
				cell_center[i + 1][j + 1][0] = cell.cell_center_x;
				cell_center[i + 1][j + 1][1] = cell.cell_center_y;
			}
		}
		vec2 cell_uv = { cell_u, cell_v };
		auto [idx_x, idx_y, min_dist] = cell(cell_uv, cell_center);
		if (x + idx_x < 0 || x + idx_x >= mapWidth || y + idx_y < 0 || y + idx_y >= mapHeight)
			return { false, -1, -1 };
		return { true, x + idx_x, y + idx_y };
	}

		

    inline std::tuple<bool, int, int> operator()(float mouseX, float mouseY) {
		return selectRegion(mouseX, mouseY);
	}
    inline RegionSelector(float fov, float width, float height, mat4x4 viewMat, mat4x4 modelMat, int mapWidth, int mapHeight, void* regions) {
		this->fov = fov;
		this->width = width;
		this->height = height;
		
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				this->viewMat[i][j] = viewMat[i][j];
				this->modelMat[i][j] = modelMat[i][j];
			}
		}

		this->mapWidth = mapWidth;
		this->mapHeight = mapHeight;
		this->regions = (RegionData*)regions;
	}
};