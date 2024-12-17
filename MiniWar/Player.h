#pragma once

class Player {
private:
	int gold{};
	int oil{};
	int electricity{};
	int labor{};
	int steel{};
	//一个地块管理器的引用
public:
	int id;
	Player();
	~Player();
	int get_gold();
	int get_electricity();
	int get_labor();
	int get_steel();
	int get_oil();
	void gold_cost(int cost);
	void oil_cost(int cost);
	void electricity_cost(int cost);
	void labor_cost(int cost);
	void steel_cost(int cost);
	void update();
};
