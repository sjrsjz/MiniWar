#pragma once
#include "../../include/json.hpp"
#include "../debug.h"
#include <initializer_list>
#include <vector>
#include <map>


class Config {
public:
	struct PlayerOrigionSource {
		double gold{};
		double oil{};
		double electricity{};
		double steel{};
	};
	struct MapSetting {
		int width{};
		int height{};
		int aiCount{};
	};

	struct Weapon {
		std::string name{};
		double damage{};
		double damageRange{};
		double attackSpeed{};
		std::pair<double, double> attackRange{}; // min, max
		std::vector<int> cost{};
		double AICost{};
		std::vector<std::vector<double>> UpLevelCost{};
	};

	struct Army {
		double cost{};
		std::vector<double> speed{};
		std::vector<std::vector<double>> UpLevelCost{};
	};

	struct DefaultRegionSetting {
		std::pair<double, double> HP{}; // min, max
		std::pair<int, int> ArmyCount{}; // min, max
		std::pair<double, double> OriginSize{};
		double CapitalHP{};
		int CapitalArmyCount{};
	};

	struct BuildingSetting {
		std::vector<std::vector<double>> BuildCost{}; // 建筑消耗
		std::vector<std::vector<double>> ReturnCost{}; // 拆除返还
		std::vector<std::vector<double>> ResourceGeneration{}; // 产出资源
		std::vector<std::vector<double>> SteadyCost{}; // 固定消耗
		std::vector<double> ProductCD{}; // 生产间隔，秒为单位
	};
	struct ResearchInstitutionSetting {
		double cost{};
		std::map<int, std::vector<double>> BuildingUpgradeCost{};
	};
	struct AIParameter {
		double A{};
		double k{};
		double t0{};
		std::vector<double> ArmyUpLevelCost{};
		std::vector<std::vector<double>> WeaponUpLevelCost{};
	};

	bool isLoaded = false;

	struct {
		PlayerOrigionSource playerOrigionSource{};
		std::map<std::string, MapSetting> mapSetting{};
		std::vector<Weapon> weapon{};
		Army army{};
		DefaultRegionSetting defaultRegionSetting{};
		std::map<std::string, BuildingSetting> buildingSetting{};
		ResearchInstitutionSetting researchInstitutionSetting{};
		std::map<std::string, AIParameter> aiParameter{};
	} m_data{};

public:
	Config(const std::string& path);
	Config() {};
	~Config();

	const PlayerOrigionSource& get_player_origion_source() const;
	const MapSetting& get_map_setting(const std::string& name) const;
	const Weapon& get_weapon_parameter(const std::string& name) const;
	const Weapon& get_weapon_parameter(int index) const;
	const Army& get_army_parameter() const;
	const DefaultRegionSetting& get_default_region_setting() const;
	const BuildingSetting& get_building_setting(const std::string& name) const;
	const ResearchInstitutionSetting& get_research_institution_setting() const;
	const AIParameter& get_AI_parameter(const std::string& level) const;
	bool is_loaded() const;
	void load(const std::string& path);
	static Config& instance_of();
};
