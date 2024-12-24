#pragma once
#include "./Point.h"

enum class Operator {
	Quit,
	Pause,
	Start,
	MapSet,
	SetPowerStation,
	SetRefinery,
	SetSteelFactory,
	SetCivilFactory,
	SetMilitaryFactory,
	RemoveBuilding,
	SetResearch,
	BuildingLevel, //region上的建筑升级
	PowerStationUpLevel,
	RefineryUpLevel,
	SteelFactoryUpLevel,
	CivilFactoryUpLevel,
	MilitaryFactoryUpLevel,
	ArmyUpLevel,
	Weapon0UpLevel,
	Weapon1UpLevel,
	Weapon2UpLevel,
	ProductArmy,
	ProductWeapon0,
	ProductWeapon1,
	ProductWeapon2,
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
	int id{};
	int size{};
	float radius{};
public:
	Operation();
	Operation(Operator op);
	Operation(int id, Operator op);
	Operation(Point cur, Operator op);
	Operation(Point cur, float radius, Operator op);
	Operation(Point start, Point end, Operator op);
	Operation(Point start, Point end, int size, Operator op);
	Point getStart();	
	Point getEnd();
	Point getCur();
	Operator getOp();
	int getSize();
	float getRadius();
	int getId();
};
