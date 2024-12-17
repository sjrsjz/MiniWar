#pragma once
#include "Player.h"
#include <vector>
<<<<<<< HEAD
class Building {
	int level;
	std::vector<int> cost;
	std::vector<int> production;
public:
	Building();
	~Building();
	bool upLevel(Player player);
=======
#include <string>
class Building {
	int level;
	std::vector<int> cost;

	//�ƽ�ʯ�͡��ֲġ�����������,Ѳ��,�г�,�޼�,ս��
	std::vector<int> production;
	std::string name;
public:
	Building(std::string);
	~Building();
	bool upLevel(Player& player);
	std::vector<int> product();
	std::vector<int> getCost();
	std::string getName();
	bool remove(Player& player);
>>>>>>> 342fef05818501dee7608a80ac7adf078302a9c5
};
