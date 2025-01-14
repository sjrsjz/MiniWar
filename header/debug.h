#pragma once
#include <sstream>
#include <string>
#include <vector>
#include <type_traits>
#include "ByteArray.h"
#include <iostream>
#include <regex>

#define ENABLE_DEBUG_OUTPUT

namespace DEBUG {
    //convert string to wstring
    inline std::wstring to_wide_string(const std::string& input)
    {

        std::mbstate_t state = std::mbstate_t();
        std::wstring ret(input.size(), 0);
        const char* data = input.data();
        wchar_t* ptr = &ret[0];
        std::locale loc;
        size_t res = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc).in(state, data, data + input.size(), data, ptr, ptr + ret.size(), ptr);
        ret.resize(ptr - &ret[0]);
        return ret;

    }

    // 默认情况：is_vector为false
    template<typename T, typename... Types>
    struct is_vector : std::false_type {};

    // 特化：为所有vector类型，is_vector为true
    template<typename T, typename... Types>
    struct is_vector<const std::vector<T, Types...>&> : std::true_type {};

    template<typename T, typename... Types>
    std::wstringstream DebugOutputString(const T& data, const Types&... args) {
        std::wstringstream ss;
        if constexpr (std::is_same<decltype(data), const DATA::ByteArray<unsigned char>&>::value) {
            ss << "size:" << data.size << "{";
            for (size_t i = 0; i < data.size; i++) {
                ss << (i ? L", " : L"") << ((unsigned char)data.ptr[i]);
            }
            ss << "}";
        }
        else if constexpr (is_vector<decltype(data)>::value) {
            ss << "size:" << data.size() << "{";
            for (size_t i = 0; i < data.size(); i++) {
                ss << (i ? L", " : L"") << DebugOutputString(data[i]).str();
            }
            ss << "}";
        }
        else {
            if constexpr (std::is_same<decltype(data), const std::string&>::value) {
                ss << to_wide_string(data);
            }
            else if constexpr (std::is_same<decltype(data), const std::wstring&>::value) {
                ss << data;
            }
            else {
                ss << data;
            }
        }
        if constexpr (sizeof...(args)) {
            ss << L" | " << DebugOutputString(args...).str();
        }
        return ss;
    }

    template<typename T, typename... Types>
    void DebugOutput(const T& data, const Types&... args) {
#if _DEBUG && defined(ENABLE_DEBUG_OUTPUT)
        std::wstring tmp = DebugOutputString(data, args...).str();
		std::wcout << tmp << std::endl;
        //OutputDebugStringW(tmp.c_str());
        //OutputDebugStringW(L"\n");
#else
        std::wstring tmp = DebugOutputString(data, args...).str();
        //std::wcout << tmp << std::endl;
#endif
    }

    inline std::string GetNamespacedFunctionName(const std::string& fullFuncName) {
        std::regex pattern(R"(([a-zA-Z0-9_:]+)\s*\([^)]*\))");
        std::smatch match;
        if (std::regex_search(fullFuncName, match, pattern) && match.size() > 1) {
            return match[1].str();
        }
        else if (fullFuncName.find("(") != std::string::npos) {
            std::regex simple_pattern(R"(([a-zA-Z0-9_]+)\s*\([^)]*\))");
            if (std::regex_search(fullFuncName, match, simple_pattern) && match.size() > 1) {
                return match[1].str();
            }
        }

        return fullFuncName;
    }
}
#ifdef _MSC_VER
#define FULL_FUNC_NAME __FUNCSIG__
#elif defined(__GNUG__)
#define FULL_FUNC_NAME __PRETTY_FUNCTION__
#else
#define FULL_FUNC_NAME __func__
#endif
#define DEBUGOUTPUT(...) DEBUG::DebugOutput(DEBUG::GetNamespacedFunctionName(FULL_FUNC_NAME) + "()", __VA_ARGS__)