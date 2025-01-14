#pragma once
#include "./utils/SmoothMove.h"
class Camera
{

	public:
		double x, y, z;
		double x_2D, y_2D;
		double pitch/*Z*/, yaw/*Y*/, roll/*X*/;
		Camera() {

			x = y = z = 0;
			x_2D = y_2D = 0;
			pitch = yaw = roll = 0;
		}
		void setPos(double x, double y, double z) {
			this->x = x;
			this->y = y;
			this->z = z;
			this->x_2D = x;
			this->y_2D = y;
		}
		void setRot(double pitch, double yaw, double roll) {
			this->pitch = pitch;
			this->yaw = yaw;
			this->roll = roll;
		}
		void rotateX(double amt) {
			pitch += amt;
		}
		void rotateY(double amt) {
			yaw += amt;
		}
		void rotateZ(double amt) {
			roll += amt;
		}

		void moveX(double amt) { x += amt; }
		void moveY(double amt) { y += amt; }
		void moveZ(double amt) { z += amt; }
		void movePitch(double amt) { pitch += amt; }
		void moveYaw(double amt) { yaw += amt; }
		void moveRoll(double amt) { roll += amt; }
		void move(double dx, double dy, double dz) {
			double m[9];
			getRotationMatrix(m);
			double dx2 = m[0] * dx + m[3] * dy + m[6] * dz;
			double dy2 = m[1] * dx + m[4] * dy + m[7] * dz;
			double dz2 = m[2] * dx + m[5] * dy + m[8] * dz;

			x += dx2;
			y += dy2;
			z += dz2;

		}
		void moveForward(double amt) {
			move(amt, 0, 0);
		}
		void getRotationMatrix(double* m) {
			//以视线方向为x轴
			//先绕y轴旋转yaw，再绕z轴旋转pitch，最后绕x轴旋转roll
			//m[0] m[1] m[2]
			//m[3] m[4] m[5]
			//m[6] m[7] m[8]
			double cx = cos(roll);
			double sx = sin(-roll);
			double cy = cos(yaw);
			double sy = sin(-yaw);
			double cz = cos(pitch);
			double sz = sin(-pitch);

			// 计算旋转矩阵的元素
			m[0] = cy * cz - sx * sy * sz; // 第一行第一列
			m[1] = -cx * sz; // 第一行第二列
			m[2] = sy * cz + sx * cy * sz; // 第一行第三列
			m[3] = cy * sz + sx * sy * cz; // 第二行第一列
			m[4] = cx * cz; // 第二行第二列
			m[5] = sy * sz - sx * cy * cz; // 第二行第三列
			m[6] = -cx * sy; // 第三行第一列
			m[7] = sx; // 第三行第二列
			m[8] = cx * cy; // 第三行第三列
			
		}
		void getMatrix(double* m) {
			//以视线方向为x轴
			// 先绕y轴旋转yaw，再绕z轴旋转pitch，最后绕x轴旋转roll
			//获得旋转后的dx,dy,dz
			//m[0] m[1] m[2] m[3]
			//m[4] m[5] m[6] m[7]
			//m[8] m[9] m[10] m[11]
			//m[12] m[13] m[14] m[15]
			double m1[9];
			getRotationMatrix(m1);
			m[0] = m1[0]; m[1] = m1[1]; m[2] = m1[2]; m[3] = 0;
			m[4] = m1[3]; m[5] = m1[4]; m[6] = m1[5]; m[7] = 0;
			m[8] = m1[6]; m[9] = m1[7]; m[10] = m1[8]; m[11] = 0;
			m[12] = -x; m[13] = -y; m[14] = -z; m[15] = 1;
		}

		void getMat4(mat4x4 m) {
			double m1[16];
			getMatrix(m1);
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					m[i][j] = m1[i * 4 + j];
				}
			}
		}
};


class SmoothCamera {
	SmoothMove x, y, z;
	SmoothMove pitch, yaw, roll;
public:
	SmoothCamera() {
		x = SmoothMove();
		y = SmoothMove();
		z = SmoothMove();
		pitch = SmoothMove();
		yaw = SmoothMove();
		roll = SmoothMove();
#define DURATION 1
		x.set_total_duration(DURATION);
		y.set_total_duration(DURATION);
		z.set_total_duration(DURATION);
		pitch.set_total_duration(DURATION);
		yaw.set_total_duration(DURATION);
		roll.set_total_duration(DURATION);
#undef DURATION
	}
	void setPos(double x, double y, double z, double time) {
		this->x.set_start_position(x, time);
		this->y.set_start_position(y, time);
		this->z.set_start_position(z, time);
	}
	void setRot(double pitch, double yaw, double roll, double time) {
		this->pitch.set_start_position(pitch, time);
		this->yaw.set_start_position(yaw, time);
		this->roll.set_start_position(roll, time);
	}

	void move(double dx, double dy, double dz, double time) {
		x.new_end_position(x.x() + dx, time);
		y.new_end_position(y.x() + dy, time);
		z.new_end_position(z.x() + dz, time);
	}
	void move_to(double x, double y, double z, double time) {
		this->x.new_end_position(x, time);
		this->y.new_end_position(y, time);
		this->z.new_end_position(z, time);
	}
	void clampX(double x_min, double x_max, double time) {
		if (x.x() > x_max) x.new_end_position(x_max, time);
		if (x.x() < x_min) x.new_end_position(x_min, time);
	}
	void clampY(double y_min, double y_max, double time) {
		if (y.x() > y_max) y.new_end_position(y_max, time);
		if (y.x() < y_min) y.new_end_position(y_min, time);
	}
	void clampZ(double z_min, double z_max, double time) {
		if (z.x() > z_max) z.new_end_position(z_max, time);
		if (z.x() < z_min) z.new_end_position(z_min, time);
	}

	void rotate(double dpitch, double dyaw, double droll, double time) {
		pitch.new_end_position(pitch.x() + dpitch, time);
		yaw.new_end_position(yaw.x() + dyaw, time);
		roll.new_end_position(roll.x() + droll, time);
	}

	void rotate_to(double pitch, double yaw, double roll, double time) {
		this->pitch.new_end_position(pitch, time);
		this->yaw.new_end_position(yaw, time);
		this->roll.new_end_position(roll, time);
	}

	void setMoveDuration(double time) {
		x.set_total_duration(time);
		y.set_total_duration(time);
		z.set_total_duration(time);
	}

	void setRotateDuration(double time) {
		pitch.set_total_duration(time);
		yaw.set_total_duration(time);
		roll.set_total_duration(time);
	}

	void update(double time) {
		x.update_sin(time);
		y.update_sin(time);
		z.update_sin(time);
		pitch.update_sin(time);
		yaw.update_sin(time);
		roll.update_sin(time);
	}

	void getCamera(Camera& camera) {
		camera.setPos(x.x(), y.x(), z.x());
		camera.setRot(pitch.x(), yaw.x(), roll.x());
	}

	Camera getCamera() {
		Camera camera;
		camera.setPos(x.x(), y.x(), z.x());
		camera.setRot(pitch.x(), yaw.x(), roll.x());
		return camera;
	}

	double getX() {
		return x.x();
	}
	double getY() {
		return y.x();
	}
	double getZ() {
		return z.x();
	}
	double getPitch() {
		return pitch.x();
	}
	double getYaw() {
		return yaw.x();
	}
	double getRoll() {
		return roll.x();
	}
};