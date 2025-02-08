#pragma once

#include <chrono>
#include <iostream>
#include <mutex>
class GlobalTimer
{
	std::chrono::steady_clock::time_point last_update_time;
	double acc_time = 0;
	bool paused = false;
	double dt = 0;
	mutable std::mutex clock_mutex;
public:
	static GlobalTimer& instance_of();

	// 获得累计时间
	double get_acc_time() const {
		std::lock_guard<std::mutex> lock(clock_mutex);
		return acc_time;
	}

	// 获得计时间隔
	double get_dt() const {
		std::lock_guard<std::mutex> lock(clock_mutex);
		return dt;
	}

	// 获得运行时间（比累计时间更精确）
	double get_running_time() const {
		std::lock_guard<std::mutex> lock(clock_mutex);
		if (paused)
			return acc_time;
		auto curr = std::chrono::steady_clock::now();
		return std::chrono::duration_cast<std::chrono::duration<double>>(curr - last_update_time).count() + acc_time;
	}

	// 更新时间
	void update() {
		std::lock_guard<std::mutex> lock(clock_mutex);
		auto curr = std::chrono::steady_clock::now();
		if (!paused) {
			dt = std::chrono::duration_cast<std::chrono::duration<double>>(curr - last_update_time).count();
			acc_time += dt;
		}
		else {
			dt = 0;
		}
		last_update_time = curr;
	}

	// 暂停计时
	void pause() {
		std::lock_guard<std::mutex> lock(clock_mutex);
		paused = true;
	}

	// 恢复计时
	void resume() {
		std::lock_guard<std::mutex> lock(clock_mutex);
		paused = false;
		last_update_time = std::chrono::steady_clock::now();
		dt = 0;
	}

	// 重置计时器
	void reset() {
		std::lock_guard<std::mutex> lock(clock_mutex);
		acc_time = 0;
		dt = 0;
		last_update_time = std::chrono::steady_clock::now();
		paused = false;
	}

};

