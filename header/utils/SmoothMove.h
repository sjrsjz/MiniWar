#include <cmath>
static const double G_PI = 3.14159265358979323846;

// �����㷨
class SmoothMove {
private:
    double v0 = 0;
    double v1 = 0;
    double x0 = 0;
    double x1 = 0;
    double dx = 0;
    double startTime = 0;
    double totalDuration = 0;

    double v = 0;

public:
    inline void newEndPosition(double x, double time) {
        x0 = x1;
        v0 = v1;
        dx = x - x0;
        startTime = time;
    }

    inline void setStartPosition(double x, double time) {
        x0 = x;
        x1 = x;
        v0 = 0;
        dx = 0;
        startTime = time;
    }

    inline void setTotalDuration(double time) {
        totalDuration = time;
    }

    inline void update(double time) {
        double t = time - startTime;
        if (t >= totalDuration) {
            x1 = x0 + dx;
            v1 = 0;
            return;
        }
        const double _1_div_exp_2 = 1.0 / std::exp(2.0);
        x1 = x0 + t * (v0 + (v0 - dx / totalDuration) / (1 - _1_div_exp_2) * (std::exp(-2 * t / totalDuration) - 1));
        v1 = ((totalDuration - 2 * t) * (v0 * totalDuration - dx) * std::exp(-2 * t / totalDuration) + totalDuration * dx - v0 * totalDuration * totalDuration * _1_div_exp_2) / (totalDuration * totalDuration * (1 - _1_div_exp_2));
        v = v1;
    }

    inline void update_sin(double time) {
        double t = time - startTime;
        if (t >= totalDuration) {
            x1 = x0 + dx;
            v1 = 0;
            return;
        }
        double t0 = t;
        t = t / totalDuration * G_PI / 2;
        t = std::sin(t) * totalDuration;
        double dt0_dt = std::cos(t0 / totalDuration * G_PI / 2) * G_PI / 2;

        const double _1_div_exp_2 = 1.0 / std::exp(2.0);
        x1 = x0 + t * (v0 + (v0 - dx / totalDuration) / (1 - _1_div_exp_2) * (std::exp(-2 * t / totalDuration) - 1));
        v1 = ((totalDuration - 2 * t) * (v0 * totalDuration - dx) * std::exp(-2 * t / totalDuration) + totalDuration * dx - v0 * totalDuration * totalDuration * _1_div_exp_2) / (totalDuration * totalDuration * (1 - _1_div_exp_2));
        v = dt0_dt * v1;
    }

    inline double getV() const {
        return v;
    }

    inline double getX() const {
        return x1;
    }

	inline void clamp(double min, double max, double time) {
		if (getX() < min) {
			newEndPosition(min, time);
		}
		if (getX() > max) {
			newEndPosition(max, time);
		}
	}
};