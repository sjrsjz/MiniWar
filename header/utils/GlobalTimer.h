#pragma once

#include <chrono>
#include <iostream>

class GlobalTimer
{
public:
	static GlobalTimer& getInstance();
	void start();
	void stop();
	void pause();
	void resume();
	double get_elapsed_time() const;
private:
	std::chrono::steady_clock::time_point start_time_;
	std::chrono::steady_clock::time_point end_time_;
	std::chrono::steady_clock::time_point pause_time_;
	std::chrono::steady_clock::duration paused_duration_;
	bool is_running_;
	bool is_paused_;

	GlobalTimer() : is_running_(false), is_paused_(false), paused_duration_(std::chrono::steady_clock::duration::zero()) {};
	~GlobalTimer() = default;
	GlobalTimer(const GlobalTimer&) = delete;
	GlobalTimer& operator=(const GlobalTimer&) = delete;
};

