#pragma once
#include "Player.h"
#include <vector>
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
};
