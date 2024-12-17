#pragma once
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
			double sx = sin(roll);
			double cy = cos(yaw);
			double sy = sin(yaw);
			double cz = cos(pitch);
			double sz = sin(pitch);

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
			m[12] = x; m[13] = y; m[14] = z; m[15] = 1;
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