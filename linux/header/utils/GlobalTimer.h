#ifndef GLOBAL_TIMER_H
#define GLOBAL_TIMER_H
#include <chrono>
#include <iostream>

class GlobalTimer {
public:
	static GlobalTimer& getInstance();
	/*void start();
	void stop();
	void pause();
	void resume();
	double get_elapsed_time() const;
	GlobalTimer() : is_running_(false), is_paused_(false), paused_duration_(std::chrono::steady_clock::duration::zero()) {};
	~GlobalTimer() = default;*/

	double get_acc_time() const {
		return acc_time;
	}

	double get_dt() const {
		return dt;
	}

	double get_running_time() const {
		if (paused)
			return acc_time;
		auto curr = std::chrono::steady_clock::now();
		return std::chrono::duration_cast<std::chrono::duration<double>>(curr - last_update_time).count() + acc_time;
	}
	void update() {
		auto curr = std::chrono::steady_clock::now();
		if (!paused) {
			dt = std::chrono::duration_cast<std::chrono::duration<double>>(curr - last_update_time).count();
			acc_time += dt;
		}
		last_update_time = curr;
	}


	void pause() {
		paused = true;
	}

	void resume() {
		paused = false;
		last_update_time = std::chrono::steady_clock::now();
	}

	void reset() {
		acc_time = 0;
		dt = 0;
		last_update_time = std::chrono::steady_clock::now();
		paused = false;
	}

private:
	//std::chrono::steady_clock::time_point start_time_;
	//std::chrono::steady_clock::time_point end_time_;
	//std::chrono::steady_clock::time_point pause_time_;
	//std::chrono::steady_clock::duration paused_duration_;
	//bool is_running_;
	//bool is_paused_;

	//GlobalTimer(const GlobalTimer&) = delete;
	//GlobalTimer& operator=(const GlobalTimer&) = delete;

	std::chrono::steady_clock::time_point last_update_time;
	double acc_time = 0;
	bool paused = false;
	double dt = 0;
};


#endif