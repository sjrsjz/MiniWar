#include <cmath>
static const double G_PI = 3.14159265358979323846;

// �����㷨
class SmoothMove {
private:
    double m_v0 = 0;
    double m_v1 = 0;
    double m_x0 = 0;
    double m_x1 = 0;
    double m_dx = 0;
    double m_start_time = 0;
    double m_total_duration = 0;
    double m_v = 0;

public:
    inline void new_end_position(double x, double time) {
        m_x0 = m_x1;
        m_v0 = m_v1;
        m_dx = x - m_x0;
        m_start_time = time;
    }

    inline void set_start_position(double x, double time) {
        m_x0 = x;
        m_x1 = x;
        m_v0 = 0;
        m_dx = 0;
        m_start_time = time;
    }

    inline void set_total_duration(double time) {
        m_total_duration = time;
    }

    inline void update(double time) {
        double t = time - m_start_time;
        if (t >= m_total_duration) {
            m_x1 = m_x0 + m_dx;
            m_v1 = 0;
            return;
        }
        const double _1_div_exp_2 = 1.0 / std::exp(2.0);
        m_x1 = m_x0 + t * (m_v0 + (m_v0 - m_dx / m_total_duration) / (1 - _1_div_exp_2) * (std::exp(-2 * t / m_total_duration) - 1));
        m_v1 = ((m_total_duration - 2 * t) * (m_v0 * m_total_duration - m_dx) * std::exp(-2 * t / m_total_duration) + m_total_duration * m_dx - m_v0 * m_total_duration * m_total_duration * _1_div_exp_2) / (m_total_duration * m_total_duration * (1 - _1_div_exp_2));
        m_v = m_v1;
    }

    inline void update_sin(double time) {
        double t = time - m_start_time;
        if (t >= m_total_duration) {
            m_x1 = m_x0 + m_dx;
            m_v1 = 0;
            return;
        }
        double t0 = t;
        t = t / m_total_duration * G_PI / 2;
        t = std::sin(t) * m_total_duration;
        double dt0_dt = std::cos(t0 / m_total_duration * G_PI / 2) * G_PI / 2;

        const double _1_div_exp_2 = 1.0 / std::exp(2.0);
        m_x1 = m_x0 + t * (m_v0 + (m_v0 - m_dx / m_total_duration) / (1 - _1_div_exp_2) * (std::exp(-2 * t / m_total_duration) - 1));
        m_v1 = ((m_total_duration - 2 * t) * (m_v0 * m_total_duration - m_dx) * std::exp(-2 * t / m_total_duration) + m_total_duration * m_dx - m_v0 * m_total_duration * m_total_duration * _1_div_exp_2) / (m_total_duration * m_total_duration * (1 - _1_div_exp_2));
        m_v = dt0_dt * m_v1;
    }

    inline double v() const {
        return m_v;
    }

    inline double x() const {
        return m_x1;
    }

	inline void clamp(double min, double max, double time) {
		if (x() < min) {
			new_end_position(min, time);
		}
		if (x() > max) {
			new_end_position(max, time);
		}
	}
};