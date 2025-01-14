#pragma once
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <cstring>
#include <optional>
#include <vector>
#include <sstream>
#include <fstream>
#include <codecvt>


namespace DATA {
    template<typename class_T = unsigned char>
    class ByteArray {
    private:
        void destroy() {
            if (!m_ptr) delete m_ptr;
            m_size = 0;
        }

    public:
        size_t m_size;
        char* m_ptr;

        ByteArray(const ByteArray& o) {
            m_size = o.m_size;
            m_ptr = (char*)new class_T[m_size];
            memcpy(m_ptr, o.m_ptr, m_size);
        }
        void operator =(const ByteArray& o) {
            destroy();
            m_size = o.m_size;
            m_ptr = (char*)new class_T[m_size];
            memcpy(m_ptr, o.m_ptr, m_size);
        }
        ByteArray(ByteArray&& o) noexcept {
            m_size = o.m_size;
            m_ptr = o.m_ptr;
            o.m_size = 0;
            o.m_ptr = nullptr;
        }
        void operator =(ByteArray&& o) noexcept {
            m_size = o.m_size;
            m_ptr = o.m_ptr;
            o.m_size = 0;
            o.m_ptr = nullptr;
        }

        ByteArray() {
            m_ptr = nullptr;
            m_size = 0;
        }
        ByteArray(const size_t length) {
            m_size = length;
            m_ptr = new char[m_size];
        }
        ~ByteArray() {
            destroy();
        }
        ByteArray attach(const ByteArray& o) {
            ByteArray t(m_size + o.m_size);
            if (m_ptr) memcpy(t.m_ptr, m_ptr, m_size);
            if (o.m_ptr) memcpy((void*)((size_t)t.m_ptr + m_size), o.m_ptr, o.m_size);
            return t;
        }
        ByteArray attach(const std::string& o) {
            ByteArray t(m_size + o.length());
            if (m_ptr) memcpy(t.m_ptr, m_ptr, m_size);
            if (o.c_str()) memcpy((void*)((size_t)t.m_ptr + m_size), o.c_str(), o.size());
            return t;
        }
        ByteArray attach(const std::wstring& o) {
            ByteArray t(m_size + o.size() * sizeof(wchar_t));
            if (m_ptr) memcpy(t.m_ptr, m_ptr, m_size);
            if (o.c_str()) memcpy((void*)((size_t)t.m_ptr + m_size), o.c_str(), o.size() * sizeof(wchar_t));
            return t;
        }
        template<typename T>void push_back(const T& o) {
            *this = attach(o);
        }
        template<typename T>ByteArray attach(const T& o) {
            ByteArray t(m_size + sizeof(T));
            if (m_ptr) memcpy(t.m_ptr, m_ptr, m_size);
            memcpy((void*)((size_t)t.m_ptr + m_size), &o, sizeof(T));
            return t;
        }
        template<typename T>ByteArray operator +(const T& o) {
            return attach(o);
        }
        template<typename T>void operator +=(const T& o) {
            push_back(o);
        }
        ByteArray& operator <<(unsigned char t) {
            push_back(t);
            return *this;
        }
        template<typename T> T& get(size_t index) {
            return *((T*)((size_t)m_ptr + index));
        }
        template<typename T> void set(size_t index, T& o) {
            memcpy((void*)((size_t)m_ptr + index), &o, sizeof(T));
        }
        class_T& operator [](size_t index) {
            return *((class_T*)((size_t)m_ptr + index));
        }
        template<typename T> T& operator [](size_t index) {
            return get<T>(index);
        }
        ByteArray sub_of(size_t offset, size_t length) {
            ByteArray t(length);
            memcpy(t.m_ptr, (void*)((size_t)m_ptr + offset), t.m_size);
            return t;
        }
        ByteArray replace(size_t offset, const ByteArray& o, std::optional<size_t> length) {
            if (length.has_value()) {
                ByteArray t(m_size + o.m_size - length.value());
                if (m_ptr) {
                    memcpy(t.m_ptr, m_ptr, offset);
                    memcpy(t.m_ptr + offset, o.m_ptr, o.m_size);
                    memcpy(t.m_ptr + offset + o.m_size, m_ptr + offset + length.value(), m_size - offset - length.value());
                }
                return t;
            }
            else {
                ByteArray t(m_size + o.m_size);
                if (m_ptr) {
                    memcpy(t.m_ptr, m_ptr, offset);
                    memcpy(t.m_ptr + offset, o.m_ptr, o.m_size);
                    memcpy(t.m_ptr + offset + o.m_size, m_ptr + offset + o.m_size, m_size - offset - o.m_size);
                }
                return t;
            }
        }
        std::string to_string() {
            std::string t;
            t.resize(m_size);
            memcpy((void*)t.data(), (void*)m_ptr, m_size);
            return t;
        }
        std::wstring to_wstring() {
            std::wstring t;
            t.resize(m_size);
            memcpy((void*)t.data(), (void*)m_ptr, m_size);
            return t;
        }
        ByteArray insert(size_t offset, const ByteArray& o) {
            ByteArray t(m_size + o.m_size);
            if (m_ptr) {
                memcpy(t.m_ptr, m_ptr, offset);
                memcpy(t.m_ptr + o.m_size, m_ptr + offset, m_size - offset);
            }
            if (o.m_ptr) memcpy(t.m_ptr + offset, o.m_ptr, o.m_size);
            return t;
        }
        bool self_insert(size_t offset, const ByteArray& o) {
            ByteArray t(m_size + o.m_size);
            if (m_ptr) {
                memcpy(t.m_ptr, m_ptr, offset);
                memcpy(t.m_ptr + o.m_size + offset, m_ptr + offset, m_size - offset);
            }
            if (o.m_ptr) memcpy(t.m_ptr + offset, o.m_ptr, o.m_size);
            std::swap(*this, t);
            return true;
        }
        template<typename T>
        ByteArray& operator = (const T& o) {
            destroy();
            m_size = sizeof(T);
            m_ptr = new char[m_size];
            memcpy(m_ptr, &o, m_size);
            return *this;
        }
    };
}
