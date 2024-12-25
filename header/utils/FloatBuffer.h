#pragma once
#include <vector>
#include <memory>

namespace DATA {
	class FloatBuffer : public std::vector<float> {
	public:
		FloatBuffer() {}
		~FloatBuffer() {}
		FloatBuffer& operator << (float t) {
			push_back(t);
			return *this;
		}

		std::unique_ptr<float[]> buffer() {
			// ����һ�� std::unique_ptr ������
			std::unique_ptr<float[]> ptr(new float[size()]);
			std::copy(begin(), end(), ptr.get());
			return ptr;
		}

	};
}