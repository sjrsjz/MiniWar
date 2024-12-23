#include "../utils/Config.h"
#include <string>
#include <iostream>
#include <vector>
/* using namespace std::string_literals; */

using json = nlohmann::json;

int main(int argc, char *argv[])
{
	Config config("../../MiniWar/config.json");
	int res = config.getConfig({"mapSize", "large", "width"});
	std::cout << res + 6 << std::endl;

	return 0;
}




