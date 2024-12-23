#include <vector>
#include <tuple>
#include <random>
#include "../../header/Logic/RegionManager.h"
#include "../../header/utils/Config.h"
#include "../../header/Logic/Player.h"

using json = nlohmann::json;

class AI {
	int gold;
	std::vector<int> weapons;
	std::tuple<int, int> capital;
	RegionManager& regionManager = RegionManager::getInstance();
	Config config = Config::getInstance();
	std::vector<int> arm_level;	
	int id;
	json increaseRate;
	std::tuple<int, int> playerCapital;
	float size;
public:
	AI() {
		// temp
		// TODO
		gold = 1000;
		weapons = { 0, 0, 0 };
		arm_level = { 1, 0, 0, 0 };
		id = 1;
		try {
			int playerCapitalX = regionManager.get_player.get_capital_x();
			int playerCapitalY = regionManager.get_player.get_capital_y();
			playerCapital = std::make_tuple(playerCapitalX, playerCapitalY);
		} catch (std::exception e) {
			throw new std::exception("Player Capital not found");
		}

		std::tuple<float, float> originSize = config.getConfig({"Region", "OriginSize"}).template get<std::tuple<float, float>>();

		
		int mapWidth = RegionManager.get_map_width();
		int mapHeight = RegionManager.get_map_height();
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> disX(3, mapWidth - 3);
		std::uniform_int_distribution<int> disY(3, mapHeight - 3);
		std::uniform_int_distribution<int> disSize(std::get<0>(originSize), std::get<1>(originSize));
		size = disSize(gen);
		while (true){
			int x = disX(gen);
			int y = disY(gen);
			bool flag = true;
			for (int i = -3; i <= 3; i++)
			{
				for (int j = -3; j <= 3; j++)
				{
					if (i*i + j*j > size*size) continue;
					if (regionManager.get_region(x + i, y + j).getOwner() != -1)
					{
						flag = false;
						break;
					}
				}
			}
			if (flag) {
				capital = std::make_tuple(x, y);
				break;
			}
		}

		for (int i = -3; i <= 3; i++)
		{
			for (int j = -3; j<= 3; j++)
			{
				if (i*i + j*j > size*size) continue;
				regionManager.get_region(std::get<0>(capital) + i, std::get<1>(capital) + j).setOwner(id);
			}
		}
		regionManager.get_region(std::get<0>(capital), std::get<1>(capital)).setMaxHp(1000);
		regionManager.get_region(std::get<0>(capital), std::get<1>(capital)).setHp(1000);
		regionManager.get_region(std::get<0>(capital), std::get<1>(capital)).getArmy().addArmy(200);

	}		

	AI(int id) {
		gold = 1000;
		weapons = { 0, 0, 0 };
		arm_level = { 1, 0, 0, 0 };
		this->id = id;
		try {
			int playerCapitalX = regionManager.get_player.get_capital_x();
			int playerCapitalY = regionManager.get_player.get_capital_y();
			playerCapital = std::make_tuple(playerCapitalX, playerCapitalY);
		} catch (std::exception e) {
			throw new std::exception("Player Capital not found");
		}

		int mapWidth = RegionManager.get_map_width();
		int mapHeight = RegionManager.get_map_height();
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> disX(1, mapWidth - 1);
		std::uniform_int_distribution<int> disY(1, mapHeight - 1);
		while (true){
			int x = disX(gen);
			int y = disY(gen);
			bool flag = true;
			for (int i = -1; i <= 1; i++)
			{
				for (int j = -1; j <= 1; j++)
				{
					if (regionManager.get_region(x + i, y + j).getOwner() != -1)
					{
						flag = false;
						break;
					}
				}
			}
			if (flag) {
				capital = std::make_tuple(x, y);
				break;
			}
		}

		for (int i = -1; i <= 1; i++)
		{
			for (int j = -1; j<= 1; j++)
			{
				regionManager.get_region(std::get<0>(capital) + i, std::get<1>(capital) + j).setOwner(id);
			}
		}
	}		
	
	


	void update() {

	}
};
