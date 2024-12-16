#pragma once
#include "../include/linmath.h"
#ifndef _Camera_h
#define _Camera_h
class Camera {
private:
	void build() {
	/*	mat4x4_identity(m);
		mat4x4_rotate_X(m, m, rot[0]);
		mat4x4_rotate_Y(m, m, rot[1]);
		mat4x4_rotate_Z(m, m, rot[2]);
		*/
	};

public:
	mat4x4 m = { {1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1} };
	vec3 pos = { 0,0,0 };
	void move(float x, float y, float z) {
		mat4x4_translate_in_place(m, x, y, z);
	}
	void rotate(float x, float y, float z) {
	}
	void update() {
//		build();
	}
	void move_local(float x, float y, float z,float rx,float ry,float rz) {
		mat4x4 m_; mat4x4_identity(m_);
		mat4x4_rotate_X(m_, m_, rx);
		mat4x4_rotate_Y(m_, m_, ry);
		mat4x4_rotate_Z(m_, m_, rz);
		
		vec4 t = { 0,0,-1,0 };
		mat4x4_mul_vec4(t, m_, t);

		pos[0] += x;
		pos[1] += y;
		pos[2] += z;

		vec3 A = {pos[0],pos[1],pos[2]};
		vec3 B = { pos[0] + t[0],pos[1] + t[1],pos[2] + t[2]};
		vec3 C = { 0,1,0 };
//		mat4x4_mul_vec4(t, m_, Vec4({ 0,0,1,1 }));
		mat4x4_look_at(m, A, B, C);
		
	}
	void move_local(vec3 p, vec3 r) {
		mat4x4 m_;
		build();
		mat4x4_translate(m_, p[0], p[1], p[2]);
		mat4x4_rotate_X(m_, m_, r[0]);
		mat4x4_rotate_Y(m_, m_, r[1]);
		mat4x4_rotate_Z(m_, m_, r[2]);
		mat4x4_mul(m, m, m_);
	}
	
};
#endif // !_Camera_h

