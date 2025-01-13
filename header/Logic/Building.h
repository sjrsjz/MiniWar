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
	int m_level = 0;
	BuildingType m_type = BuildingType::None;
public:
	Building(BuildingType);
	~Building();
	bool up_level(int max_level);
	BuildingType get_type();
	int get_level();
	bool remove();
};
