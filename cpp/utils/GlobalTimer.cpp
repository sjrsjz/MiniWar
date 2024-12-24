#include "../../header/utils/GlobalTimer.h"

GlobalTimer& GlobalTimer::getInstance()
{
	static GlobalTimer instance;
	return instance;
}

void GlobalTimer::start() {
	is_running_ = true;
	start_time_ = std::chrono::steady_clock::now();
}

void GlobalTimer::stop() {
	is_running_ = false;
	end_time_ = std::chrono::steady_clock::now();
}

void GlobalTimer::pause() {
	if (is_running_ && !is_paused_) {
		pause_time_ = std::chrono::steady_clock::now();
		is_paused_ = true;
	}
}

void GlobalTimer::resume() {
	if (is_running_ && is_paused_) {
		paused_duration_ += std::chrono::steady_clock::now() - pause_time_;
		is_paused_ = false;
	}
}

double GlobalTimer::get_elapsed_time() const{
	if (is_running_) {
		auto end_time = is_paused_ ? pause_time_ : std::chrono::steady_clock::now();
		return std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time_ - paused_duration_).count();
	}
	else {
		return std::chrono::duration_cast<std::chrono::duration<double>>(end_time_ - start_time_ - paused_duration_).count();
	}
}