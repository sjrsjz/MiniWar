#include "../utils/Config.h"
#include <string>
#include <iostream>
#include <vector>
/* using namespace std::string_literals; */

using json = nlohmann::json;

int main(int argc, char *argv[])
{
	Config config("../../MiniWar/config.json");
	/* std::cout << config.getConfig(std::string("mapSize"), std::string("large"), std::string("width")) << std::endl; */
	/* std::cout << config.getConfig("mapSize") << std::endl; */
	int res = config.getConfig({"mapSize", "large", "width"});
	std::cout << res + 6 << std::endl;

	/* json j = config.getConfig(); */
	/* std::string a = "mapSize"; */
	/* std::cout << j[a]; */
	/* std::cout << config.getConfig()["mapSize"]["large"]["width"] << std::endl; */
	return 0;
}




