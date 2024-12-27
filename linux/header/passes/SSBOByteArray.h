#ifndef SSBO_BYTE_ARRAY_H
#define SSBO_BYTE_ARRAY_H
#include "../ByteArray.h"
#include "../../include/linmath.h"
namespace DATA {
	template<typename class_T = unsigned char>
	class SSBOByteArray : public ByteArray<class_T> {
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

		SSBOByteArray& operator <<(float t) {
			ByteArray<class_T>::push_back(t);
			return *this;
		}
	};
}
#endif