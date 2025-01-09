#pragma once
#include <vector>
#include <string>

enum BuildingType {
	CivilFactory,
	MilitaryFactory,
	PowerStation,
	Refinery,
	SteelFactory,
	None,
};


inline std::string BuildingTypeToString(BuildingType type) {
	switch (type) {
	case None:
		return "None";
	case CivilFactory:
		return "CivilFactory";
	case MilitaryFactory:
		return "MilitaryFactory";
	case PowerStation:
		return "PowerStation";
	case Refinery:
		return "Refinery";
	case SteelFactory:
		return "SteelFactory";
	}
}

class Building {
	int level = 0;
	BuildingType type = BuildingType::None;
public:
	Building(BuildingType);
	~Building();
	bool upLevel(int MaxLevel);
	BuildingType getType();
	int getLevel();
	bool remove();
};
