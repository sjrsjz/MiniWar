#pragma once

#include <chrono>
#include <iostream>

class GlobalTimer
{
public:
	static GlobalTimer& instance_of();

	// 获得累计时间
	double get_acc_time() const {
		return acc_time;
	}

	// 获得计时间隔
	double get_dt() const {
		return dt;
	}

	// 获得运行时间（比累计时间更精确）
	double get_running_time() const {
		if (paused)
			return acc_time;
		auto curr = std::chrono::steady_clock::now();
		return std::chrono::duration_cast<std::chrono::duration<double>>(curr - last_update_time).count() + acc_time;
	}

	// 更新时间
	void update() {
		auto curr = std::chrono::steady_clock::now();
		if (!paused) {
			dt = std::chrono::duration_cast<std::chrono::duration<double>>(curr - last_update_time).count();
			acc_time += dt;
		}
		last_update_time = curr;
	}

	// 暂停计时
	void pause() {
		paused = true;
	}

	// 恢复计时
	void resume() {
		paused = false;
		last_update_time = std::chrono::steady_clock::now();
	}

	// 重置计时器
	void reset() {
		acc_time = 0;
		dt = 0;
		last_update_time = std::chrono::steady_clock::now();
		paused = false;
	}

private:
	std::chrono::steady_clock::time_point last_update_time;
	double acc_time = 0;
	bool paused = false;
	double dt = 0;
};

