#include "../../header/utils/Config.h"
#include <fstream>
#include <initializer_list>
#include <vector>

using json = nlohmann::json;

Config::Config(const std::string& path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		throw std::runtime_error("Could not open file: " + path);
	}
	try {
		file >> config;
		tmp = config;
	} catch (json::parse_error& e) {
		throw std::runtime_error("Could not parse file: " + path);
	}
	file.close();
}

Config::~Config() {}

const json& Config::getConfig() {
	return config;
}

/* template<typename T, typename... Args> */
/* const json Config::getConfig(const T& first, const Args&... args) { */
/* 	try { */
/* 		const json* tmp = &config; */
/* 		getKey(tmp, std::string(first)); */
/* 		(getKey(tmp, std::string(args)), ...); */
/* 		return *tmp; */
/* 		/1* return tmp->get<json>(); *1/ */
/* 		/1* if constexpr (std::is_same<T, nlohmann::json>::value) { *1/ */
/* 		/1* return *tmp; *1/ */
/* 	/1* } else { *1/ */
/* 		/1* return tmp->get<T>(); *1/ */
/* 	/1* } *1/ */
/* 	} catch (std::out_of_range& e) { */
/* 		throw std::runtime_error(e.what()); */
/* 	} */
/* } */


/* void Config::getKey(const json*& tmp, const std::string& key) { */
/* 	if (tmp->find(key) == tmp->end()) { */
/* 		throw std::out_of_range("Key not found: " + key); */
/* 	} */
/* 	tmp = &(*tmp)[key]; */
/* } */


/* template<typename T> */
const json Config::getConfig(std::initializer_list<std::string> args) {
/* const json Config::getConfig(std::string s){ */
	/* return this->config[s]; */
	try {
		json tmp = config;
		for (auto& arg : args) {
			std::string s =	arg;
			if (tmp.find(s) == tmp.end()) {
				throw std::out_of_range("Key not found: " + arg);
			} else {
				tmp = tmp[s];
			}
		}
		return tmp;
	} catch (std::out_of_range& e) {
		throw std::runtime_error(e.what());
	}	
}
