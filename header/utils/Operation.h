#pragma once
#include "./Point.h"

enum class Operator {
	Quit,
	Pause,
	Start,
	MapSet,
	SetBuilding,
	RemoveBuilding,
	SetResearch,
	BuildingUpLevel,
	ResearchUpLevel,
	WeaponUpLevel,
	Product,
	ArmyMove,
	Weapon0Attack,
	Weapon1Attack,
	Weapon2Attack,
	RangeAttack,
};

class Operation{
	Point start;
	Point end;
	Point cur;
	Operator op;
	int size{};
	float radius{};
public:
	Operation();
	Operation(Operator op);
	Operation(int size, Operator op);
	Operation(Point cur, Operator op);
	Operation(Point cur, float radius, Operator op);
	Operation(Point start, Point end, Operator op);
	Point getStart();	
	Point getEnd();
	Point getCur();
	Operator getOp();
	int getSize();
	float getRadius();
};
