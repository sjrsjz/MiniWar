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
	~Config();
	/* template<typename T, typename... Args> */
	/* const json getConfig(const T& first, const Args&... args); */
	/* template<typename T> */
	const json getConfig(std::initializer_list<std::string> args);
	const json getConfig(std::string s);
	const json& getConfig();
};
