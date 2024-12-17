#pragma once
#include "../ByteArray.h"
#include "../../include/linmath.h"
namespace DATA {
    template<typename class_T = unsigned char>
    class SSBOByteArray : public ByteArray<class_T>{
    public:
		SSBOByteArray() : ByteArray<class_T>() {}
		SSBOByteArray(const SSBOByteArray& o) : ByteArray<class_T>(o) {}
		SSBOByteArray(SSBOByteArray&& o) noexcept : ByteArray<class_T>(o) {}
		SSBOByteArray& operator =(const SSBOByteArray<class_T>& o) {
			ByteArray<class_T>::operator =(o);
			return *this;
		}
		SSBOByteArray& operator =(const ByteArray<class_T>& o) {
			ByteArray<class_T>::operator =(o);
			return *this;
		}
		SSBOByteArray& operator =(SSBOByteArray<class_T>&& o) noexcept {
			ByteArray<class_T>::operator =(std::move(o));
			return *this;
		}
		SSBOByteArray& operator =(ByteArray<class_T>&& o) noexcept {
			ByteArray<class_T>::operator =(std::move(o));
			return *this;
		}
		SSBOByteArray(const size_t length) : ByteArray<class_T>(length) {}

        void push_back(const vec4& o) {
			SSBOByteArray tmp(sizeof(vec4));
			memcpy(tmp.ptr, &o, sizeof(vec4));
			*this = *this + tmp;
        }
        void push_back(const vec3& o) {
			SSBOByteArray tmp(sizeof(vec3));
            memcpy(tmp.ptr, &o, sizeof(vec3));
            *this = *this + tmp;
        }
        void push_back(const vec2& o) {
			SSBOByteArray tmp(sizeof(vec2));
            memcpy(tmp.ptr, &o, sizeof(vec2));
            *this = *this + tmp;
        }
        void push_back(const mat4x4& o) {
			SSBOByteArray tmp(sizeof(vec4) * 4);
			for (int i = 0; i < 4; i++) {
				memcpy(tmp.ptr + i * sizeof(vec4), &o[i], sizeof(vec4));
			}
			*this = *this + tmp;
        }
		void push_back(const std::initializer_list<float> &o) {
			SSBOByteArray tmp(sizeof(float) * o.size());
			for (int i = 0; i < o.size(); i++) {
				*(float*)(tmp.ptr + i * sizeof(float)) = *(o.begin() + i);
			}
			*this = *this + tmp;
		}
    };
}
