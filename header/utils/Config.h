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
		std::vector<double> BuildCost{};
		std::vector<double> Product{};
		std::vector<double> LevelUpscaleFactor{};
		std::vector<std::vector<double>> UpLevelCost{};
		std::vector<double> ProductCD{}; // 生产间隔，秒为单位
	};
	struct ResearchInstitutionSetting {
		double cost{};
	};
	struct AIParameter {
		double A{};
		double k{};
		double t0{};
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
	} data;

public:
	Config(const std::string& path);
	Config() {};
	~Config();

	const PlayerOrigionSource& getPlayerOrigionSource() const;
	const MapSetting& getMapSetting(const std::string& name) const;
	const Weapon& getWeapon(const std::string& name) const;
	const Weapon& getWeapon(int index) const;
	const Army& getArmy() const;
	const DefaultRegionSetting& getDefaultRegionSetting() const;
	const BuildingSetting& getBuildingSetting(const std::string& name) const;
	const ResearchInstitutionSetting& getResearchInstitutionSetting() const;
	const AIParameter& getAIParameter(const std::string& level) const;
	bool is_loaded() const;
	void load(const std::string& path);
	static Config& getInstance();
};
