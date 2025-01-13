#include "../../header/utils/Config.h"
#include <fstream>
#include <initializer_list>
#include <vector>

using json = nlohmann::json;

void Config::load(const std::string& path) {
	DEBUGOUTPUT("Loading config from ", path);
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

	m_data.playerOrigionSource.electricity = config["PlayerOrigionSource"]["electricity"].get<double>();
	m_data.playerOrigionSource.gold = config["PlayerOrigionSource"]["gold"].get<double>();
	m_data.playerOrigionSource.oil = config["PlayerOrigionSource"]["oil"].get<double>();
	m_data.playerOrigionSource.steel = config["PlayerOrigionSource"]["steel"].get<double>();

	m_data.mapSetting.clear();

	for (auto& mapSetting : config["MapSetting"]) {
		m_data.mapSetting[mapSetting["name"].get<std::string>()] =
			MapSetting{
				mapSetting["width"].get<int>(),
				mapSetting["height"].get<int>(),
				mapSetting["aiCount"].get<int>()
			};
	}

	for (auto& weapon : config["Weapon"]) {
		m_data.weapon.push_back(
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

	m_data.army.cost = config["Army"]["cost"].get<double>();
	m_data.army.speed = config["Army"]["speed"].get<std::vector<double>>();
	m_data.army.UpLevelCost = config["Army"]["UpLevelCost"].get<std::vector<std::vector<double>>>();

	m_data.defaultRegionSetting.HP = std::make_pair(config["Region"]["HP"][0].get<double>(), config["Region"]["HP"][1].get<double>());
	m_data.defaultRegionSetting.ArmyCount = std::make_pair(config["Region"]["ArmyCount"][0].get<int>(), config["Region"]["ArmyCount"][1].get<int>());
	m_data.defaultRegionSetting.OriginSize = std::make_pair(config["Region"]["OriginSize"][0].get<double>(), config["Region"]["OriginSize"][1].get<double>());
	m_data.defaultRegionSetting.CapitalHP = config["Region"]["CapitalHP"].get<double>();
	m_data.defaultRegionSetting.CapitalArmyCount = config["Region"]["CapitalArmyCount"].get<int>();


	for (auto& item: config["Building"].items()){
		m_data.buildingSetting[item.key()] =
			BuildingSetting{
				item.value()["BuildCost"].get<std::vector<std::vector<double>>>(),
				item.value()["ReturnCost"].get<std::vector<std::vector<double>>>(),
				item.value()["ResourceGeneration"].get<std::vector<std::vector<double>>>(),
				item.value()["SteadyCost"].get<std::vector<std::vector<double>>>(),
				item.value()["CD"].get<std::vector<double>>()
		};
	}

	m_data.researchInstitutionSetting.cost = config["ResearchInstitution"]["BuildCost"].get<double>();

	for (auto& item : config["AIParameter"].items()) {
		m_data.aiParameter[item.key()] = {
				item.value()["A"].get<double>(),
				item.value()["k"].get<double>(),
				item.value()["t0"].get<double>(),
			};
	}


	DEBUGOUTPUT("Config loaded");
	isLoaded = true;

}

Config::Config(const std::string& path) {
	load(path);
}
Config::~Config() {}

Config& Config::instance_of() {
	static Config instance;
	if (!instance.is_loaded()) {
		instance.load("./config.json");
	}
	return instance;
}

bool Config::is_loaded() const {
	return isLoaded;
}

const Config::PlayerOrigionSource& Config::get_player_origion_source() const {
	return m_data.playerOrigionSource;
}

const Config::MapSetting& Config::get_map_setting(const std::string& name) const {
	return m_data.mapSetting.at(name);
}

const Config::Weapon& Config::get_weapon_parameter(const std::string& name) const {
	for (auto& weapon : m_data.weapon) {
		if (weapon.name == name) {
			return weapon;
		}
	}
	throw std::invalid_argument("Weapon not found: " + name);
}

const Config::Weapon& Config::get_weapon_parameter(int id) const {
	if (id >= 0 && id < m_data.weapon.size())
		return m_data.weapon.at(id);
	throw std::invalid_argument("Weapon not found: " + std::to_string(id));
}

const Config::Army& Config::get_army_parameter() const {
	return m_data.army;
}

const Config::DefaultRegionSetting& Config::get_default_region_setting() const {
	return m_data.defaultRegionSetting;
}
const Config::BuildingSetting& Config::get_building_setting(const std::string& name) const {
	return m_data.buildingSetting.at(name);
}
const Config::ResearchInstitutionSetting& Config::get_research_institution_setting() const {
	return m_data.researchInstitutionSetting;
}

const Config::AIParameter& Config::get_AI_parameter(const std::string& level) const {
	return m_data.aiParameter.at(level);
}
