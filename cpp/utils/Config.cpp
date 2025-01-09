#include "../../header/utils/Config.h"
#include <fstream>
#include <initializer_list>
#include <vector>

using json = nlohmann::json;

Config::Config(const std::string& path) {
	DEBUG::DebugOutput("Loading config from ", path);
	std::ifstream file(path);
	nlohmann::json config;
	if (!file.is_open()) {
		throw std::runtime_error("Could not open file: " + path);
	}
	try {
		file >> config;
	} catch (json::parse_error& e) {
		throw std::runtime_error("Could not parse file: " + path);
	}
	file.close();

	data.playerOrigionSource.electricity = config["PlayerOrigionSource"]["electricity"].get<double>();
	data.playerOrigionSource.gold = config["PlayerOrigionSource"]["gold"].get<double>();
	data.playerOrigionSource.oil = config["PlayerOrigionSource"]["oil"].get<double>();
	data.playerOrigionSource.steel = config["PlayerOrigionSource"]["steel"].get<double>();

	for (auto& mapSetting : config["MapSetting"]) {
		data.mapSetting[mapSetting["name"].get<std::string>()] =
			MapSetting{
				mapSetting["width"].get<int>(),
				mapSetting["height"].get<int>(),
				mapSetting["aiCount"].get<int>()
			}
		;
	}

	for (auto& weapon : config["Weapon"]) {
		data.weapon.push_back(
			Weapon{
				weapon["name"].get<std::string>(),
				weapon["damage"].get<double>(),
				weapon["damageRange"].get<double>(),
				weapon["attackSpeed"].get<double>(),
				std::make_pair(weapon["attackRange"][0].get<double>(), weapon["attackRange"][1].get<double>()),
				weapon["cost"].get<std::vector<int>>(),
				weapon["AICost"].get<double>(),
				weapon["UpLevelCost"].get<std::vector<std::vector<double>>>()
			}
		);
	}

	for (auto& item: config["Building"].items()){
		data.buildingSetting[item.key()] =
			BuildingSetting{
				item.value()["BuildCost"].get <std::vector<double>>(),
				item.value()["Product"].get <std::vector<double>>(),
				item.value()["UpLevelFactor"].get<std::vector<double>>(),
				item.value()["UpLevelCost"].get<std::vector<std::vector<double>>>(),
				item.value()["CD"].get<std::vector<double>>(),
		};
	}

	data.researchInstitutionSetting.cost = config["ResearchInstitution"]["BuildCost"].get<double>();

	for (auto& item : config["AIParameter"].items()) {
		data.aiParameter[item.key()] = {
				item.value()["A"].get<double>(),
				item.value()["k"].get<double>(),
				item.value()["t0"].get<double>(),
			};
	}


	DEBUG::DebugOutput("Config loaded");
}

Config::~Config() {}

static Config instance{};
Config& Config::getInstance() {
	if (instance.is_loaded()) {
		instance.load("./config.json");
	}
	return instance;
}

bool Config::is_loaded() const {
	return isLoaded;
}

const Config::PlayerOrigionSource& Config::getPlayerOrigionSource() const {
	return data.playerOrigionSource;
}

const Config::MapSetting& Config::getMapSetting(const std::string& name) const {
	return data.mapSetting.at(name);
}

const Config::Weapon& Config::getWeapon(const std::string& name) const {
	for (auto& weapon : data.weapon) {
		if (weapon.name == name) {
			return weapon;
		}
	}
	throw std::invalid_argument("Weapon not found: " + name);
}

const Config::Weapon& Config::getWeapon(int id) const {
	if (id >= 0 && id < data.weapon.size())
		return data.weapon.at(id);
	throw std::invalid_argument("Weapon not found: " + std::to_string(id));
}

const Config::Army& Config::getArmy() const {
	return data.army;
}

const Config::DefaultRegionSetting& Config::getDefaultRegionSetting() const {
	return data.defaultRegionSetting;
}
const Config::BuildingSetting& Config::getBuildingSetting(const std::string& name) const {
	return data.buildingSetting.at(name);
}
const Config::ResearchInstitutionSetting& Config::getResearchInstitutionSetting() const {
	return data.researchInstitutionSetting;
}

const Config::AIParameter& Config::getAIParameter(const std::string& level) const {
	return data.aiParameter.at(level);
}
