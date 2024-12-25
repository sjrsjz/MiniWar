#include "../../header/utils/Config.h"
#include <fstream>
#include <initializer_list>
#include <vector>

using json = nlohmann::json;

Config::Config(const std::string& path) {
	DEBUG::DebugOutput("Loading config from ", path);
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
	DEBUG::DebugOutput("Config loaded");
}

Config::~Config() {}

const json& Config::getConfig() {
	return config;
}


const json Config::getConfig(std::initializer_list<std::string> args) {
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

const json Config::getMapSize() {
	return getConfig({"mapSize"});
}

const json Config::getWeapons() {
	return getConfig({"Weapon"});
}

const json Config::getRegions() {
	return getConfig({"Region"});
}

const json Config::getBuildings() {
	return getConfig({"Building"});
}

const json Config::getResearch() {
	return getConfig({"ResearchInstitution"});
}
static Config instance("./config.json");
Config& Config::getInstance() {
	if (instance.getConfig().empty()) {
		instance = Config("./config.json");
	}
	return instance;
}
