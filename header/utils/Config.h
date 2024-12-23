#include "../../include/json.hpp"
#include <initializer_list>
#include <vector>

using json = nlohmann::json;

class Config {
	json config;		
	json tmp;
	void getKey(const json*& tmp, const std::string& key);
public:
	Config(const std::string& path);
	inline Config() {};
	~Config();
	const json getMapSize();
	const json getWeapons();
	const json getRegions();
	const json getBuildings();
	const json getResearch();
	const json getConfig(std::initializer_list<std::string> args);
	const json& getConfig();
	static Config& getInstance();
};
